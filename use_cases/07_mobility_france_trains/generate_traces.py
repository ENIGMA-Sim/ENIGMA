#!/usr/bin/env python3
"""
generate_traces.py - synthesize GPS traces for the France high-speed-rail
mobility use case.

Each train follows a polyline of **real SNCF station coordinates** along one of
the main LGV (Ligne a Grande Vitesse) corridors, walked at TGV cruising speed
and sampled once per second.

Output: one CSV per train in ./coords/ . The filename stem must match the
SimGrid host id in france_rail_platform.xml exactly.

  columns: timestamp,latitude,longitude,speed,heading,pax_load,delay_s

Only timestamp/latitude/longitude are required by ENIGMA; the rest are extra
columns - linearly interpolated and shown in the interactive-map popups.

Standard library only. Re-run any time; edit ROUTES to change the scenario.
"""
from __future__ import annotations

import csv
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(HERE, "coords")

DURATION_S = 7200          # trace length (s) - ~2 h of running
STEP_S = 1                 # sample period (s)

M_PER_DEG_LAT = 111_320.0


def _m_per_deg_lon(lat_deg: float) -> float:
    return 111_320.0 * math.cos(math.radians(lat_deg))


# --------------------------------------------------------------------------- #
# Real SNCF corridors: (station, lat, lon) waypoints along the LGV alignment.
# --------------------------------------------------------------------------- #
TGV_SUD_EST = [   # Paris Gare de Lyon -> Lyon -> Marseille St-Charles
    (48.84437, 2.37344),   # Paris Gare de Lyon
    (46.76762, 4.39360),   # Le Creusot - Montceau - Montchanin TGV
    (46.27971, 4.79062),   # Macon-Loche TGV
    (45.76054, 4.85953),   # Lyon Part-Dieu
    (44.99114, 4.97601),   # Valence TGV
    (43.92192, 4.78606),   # Avignon TGV
    (43.45535, 5.31763),   # Aix-en-Provence TGV
    (43.30370, 5.38030),   # Marseille St-Charles
]

TGV_ATLANTIQUE = [   # Paris Montparnasse -> Bordeaux St-Jean
    (48.84044, 2.31975),   # Paris Montparnasse
    (48.72555, 2.25970),   # Massy TGV
    (47.38541, 0.71670),   # St-Pierre-des-Corps (Tours)
    (46.58340, 0.33350),   # Poitiers
    (45.65600, 0.15670),   # Angouleme
    (44.82584, -0.55663),  # Bordeaux St-Jean
]

TGV_NORD = [   # Paris Gare du Nord -> Lille Europe -> Calais-Frethun
    (48.88094, 2.35528),   # Paris Gare du Nord
    (49.85690, 2.83220),   # Haute-Picardie TGV
    (50.63940, 3.07570),   # Lille Europe
    (50.91470, 1.81180),   # Calais-Frethun
]

TGV_EST = [   # Paris Gare de l'Est -> Reims -> Strasbourg
    (48.87660, 2.35910),   # Paris Gare de l'Est
    (49.21570, 4.00160),   # Champagne-Ardenne TGV (Reims)
    (48.98470, 5.27170),   # Meuse TGV
    (48.94900, 6.16990),   # Lorraine TGV
    (48.58500, 7.73460),   # Strasbourg
]

TGV_RHIN_RHONE = [   # Dijon Ville -> Besancon -> Belfort -> Mulhouse Ville
    (47.32350, 5.02730),   # Dijon Ville
    (47.30790, 5.95600),   # Besancon Franche-Comte TGV
    (47.58600, 6.89900),   # Belfort-Montbeliard TGV
    (47.74170, 7.34280),   # Mulhouse Ville
]

# name -> dict(waypoints, cruise m/s, phase for the load curve)
ROUTES = {
    "tgv_sud_est":     dict(wp=TGV_SUD_EST,     cruise=92.0, phase=0.0),
    "tgv_atlantique":  dict(wp=TGV_ATLANTIQUE,  cruise=88.0, phase=1.3),
    "tgv_nord":        dict(wp=TGV_NORD,        cruise=83.0, phase=2.1),
    "tgv_est":         dict(wp=TGV_EST,         cruise=87.0, phase=3.4),
    "tgv_rhin_rhone":  dict(wp=TGV_RHIN_RHONE,  cruise=78.0, phase=4.7),
}


def _seg_len_m(a, b):
    dn = (b[0] - a[0]) * M_PER_DEG_LAT
    de = (b[1] - a[1]) * _m_per_deg_lon(a[0])
    return math.hypot(dn, de)


def _point_at(wp, dist_m):
    """Ping-pong along the polyline at arc length dist_m (metres)."""
    lengths = [_seg_len_m(wp[i], wp[i + 1]) for i in range(len(wp) - 1)]
    total = sum(lengths)
    if total == 0:
        return wp[0]
    cycle = 2 * total
    d = dist_m % cycle
    if d > total:
        d = cycle - d
    for i, seg in enumerate(lengths):
        if d <= seg or i == len(lengths) - 1:
            frac = (d / seg) if seg > 0 else 0.0
            lat = wp[i][0] + frac * (wp[i + 1][0] - wp[i][0])
            lon = wp[i][1] + frac * (wp[i + 1][1] - wp[i][1])
            return (lat, lon)
        d -= seg
    return wp[-1]


def build_trace(cfg) -> list[dict]:
    wp, cruise, phase = cfg["wp"], cfg["cruise"], cfg["phase"]
    rows: list[dict] = []
    prev = None
    for t in range(0, DURATION_S + 1, STEP_S):
        lat, lon = _point_at(wp, cruise * t)

        if prev is None:
            speed, heading = cruise, 0.0
        else:
            dn = (lat - prev[0]) * M_PER_DEG_LAT
            de = (lon - prev[1]) * _m_per_deg_lon(lat)
            speed = math.hypot(dn, de) / STEP_S
            heading = (math.degrees(math.atan2(de, dn)) + 360.0) % 360.0

        pax_load = min(100.0, max(15.0, 60.0 + 35.0 * math.sin(t / 900.0 + phase)))
        # small drifting delay that grows on the return leg
        delay_s = max(0.0, 90.0 * math.sin(t / 1600.0 + phase) + 30.0)

        rows.append({
            "timestamp": t,
            "latitude": round(lat, 6),
            "longitude": round(lon, 6),
            "speed": round(speed, 2),
            "heading": round(heading, 1),
            "pax_load": round(pax_load, 1),
            "delay_s": round(delay_s, 0),
        })
        prev = (lat, lon)
    return rows


def main() -> None:
    os.makedirs(OUT_DIR, exist_ok=True)
    fields = ["timestamp", "latitude", "longitude", "speed", "heading", "pax_load", "delay_s"]
    for name, cfg in ROUTES.items():
        rows = build_trace(cfg)
        path = os.path.join(OUT_DIR, f"{name}.csv")
        with open(path, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=fields)
            w.writeheader()
            w.writerows(rows)
        km = sum(_seg_len_m(cfg["wp"][i], cfg["wp"][i + 1])
                 for i in range(len(cfg["wp"]) - 1)) / 1000.0
        print(f"  {name:16s} {len(rows)} rows  route ~{km:6.1f} km  -> {path}")
    print(f"Done - {len(ROUTES)} SNCF corridors in {OUT_DIR}/")


if __name__ == "__main__":
    main()
