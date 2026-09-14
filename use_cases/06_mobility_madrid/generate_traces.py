#!/usr/bin/env python3
"""
generate_traces.py - synthesise GPS traces for the Madrid public-transport
mobility use case.

Each device follows a polyline of **real Madrid coordinates** (bus corridors,
metro / Cercanías lines, drone patrol loops). The polyline is walked at a
per-vehicle cruise speed and sampled once per second.

Output: one CSV per device in ./coords/ . The filename stem must match the
SimGrid host id in madrid_transport_platform.xml exactly.

  buses / trains :  timestamp,latitude,longitude,speed,heading,occupancy
  drones         :  timestamp,latitude,longitude,speed,heading,altitude,battery

Only timestamp/latitude/longitude are required by ENIGMA; the extra columns are
linearly interpolated and shown in the interactive-map popups.

Standard library only. Re-run any time; edit ROUTES to change the scenario.
"""
from __future__ import annotations

import csv
import math
import os

HERE = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(HERE, "coords")

DURATION_S = 1200          # trace length (s)
STEP_S = 1                 # sample period (s)

M_PER_DEG_LAT = 111_320.0


def _m_per_deg_lon(lat_deg: float) -> float:
    return 111_320.0 * math.cos(math.radians(lat_deg))


# --------------------------------------------------------------------------- #
# Real Madrid routes  (lat, lon) waypoints, roughly following the streets /
# rail alignments of the named line.
# --------------------------------------------------------------------------- #
BUS_27 = [  # EMT 27: Plaza de Castilla <-> Embajadores  (Castellana / Prado axis)
    (40.46650, -3.68920), (40.45900, -3.69050), (40.45310, -3.68830),
    (40.44600, -3.69240), (40.43800, -3.69100), (40.43300, -3.68890),
    (40.42580, -3.69200), (40.41940, -3.69280), (40.41600, -3.69450),
    (40.40880, -3.69280), (40.40650, -3.68950), (40.40580, -3.70100),
    (40.40450, -3.70300),
]

BUS_002 = [  # EMT 2: Manuel Becerra <-> Arguelles  (Alcala + Gran Via)
    (40.42870, -3.66900), (40.42430, -3.67730), (40.42230, -3.67900),
    (40.42000, -3.68850), (40.41930, -3.69310), (40.41850, -3.69500),
    (40.41800, -3.69970), (40.42000, -3.70170), (40.42000, -3.70580),
    (40.42130, -3.70850), (40.42340, -3.71220), (40.42880, -3.71500),
]

BUS_046 = [  # EMT 46: Atocha <-> Moncloa  (Prado / Gran Via / Princesa)
    (40.40650, -3.68950), (40.41590, -3.69410), (40.41930, -3.69310),
    (40.42030, -3.69800), (40.42000, -3.70580), (40.42340, -3.71220),
    (40.42700, -3.71400), (40.43050, -3.71600), (40.43510, -3.71900),
]

METRO_L1 = [  # Metro L1: Chamartin -> Pacifico
    (40.47260, -3.68250), (40.46650, -3.68870), (40.46100, -3.69600),
    (40.45880, -3.69870), (40.45560, -3.70170), (40.44990, -3.70400),
    (40.44680, -3.70380), (40.44160, -3.70130), (40.43540, -3.69960),
    (40.42930, -3.70140), (40.42560, -3.70090), (40.42010, -3.70160),
    (40.41690, -3.70350), (40.41270, -3.70300), (40.41170, -3.69860),
    (40.40850, -3.69250), (40.40620, -3.68930), (40.40830, -3.67750),
    (40.40120, -3.67740),
]

METRO_L6 = [  # Metro L6 (Circular), clockwise ring
    (40.44680, -3.70380), (40.44590, -3.69050), (40.43720, -3.67960),
    (40.43730, -3.67700), (40.43370, -3.67730), (40.42870, -3.66900),
    (40.42310, -3.67060), (40.41780, -3.67160), (40.41140, -3.67390),
    (40.40120, -3.67740), (40.39860, -3.68190), (40.39240, -3.68730),
    (40.39100, -3.69510), (40.38560, -3.70610), (40.38520, -3.71830),
    (40.38610, -3.72770), (40.39320, -3.71810), (40.40920, -3.72320),
    (40.42060, -3.72030), (40.43080, -3.71570), (40.43510, -3.71900),
    (40.44000, -3.71130), (40.44360, -3.70960), (40.44680, -3.70380),
]

CERCANIAS_C1 = [  # Cercanias C-1: Principe Pio -> Aeropuerto T4
    (40.42060, -3.72030), (40.39830, -3.71350), (40.39680, -3.69340),
    (40.39860, -3.68190), (40.40620, -3.68930), (40.42350, -3.69220),
    (40.44590, -3.69050), (40.47260, -3.68250), (40.48060, -3.65670),
    (40.49200, -3.59200),
]

DRONE_RETIRO = [  # patrol loop: Retiro park + museum mile
    (40.42000, -3.68840), (40.42300, -3.67900), (40.41100, -3.67900),
    (40.40800, -3.69000), (40.41600, -3.69450), (40.41930, -3.69310),
    (40.42000, -3.68840),
]

DRONE_MADRID_RIO = [  # patrol loop along the Manzanares / Madrid Rio
    (40.43100, -3.72900), (40.42060, -3.72030), (40.41320, -3.72070),
    (40.40300, -3.71780), (40.39100, -3.69850), (40.39600, -3.70600),
    (40.41800, -3.72300), (40.43100, -3.72900),
]

# name -> dict(waypoints, cruise m/s, mode, kind, extra-column generator)
ROUTES = {
    "bus_27":       dict(wp=BUS_27,        cruise=8.0,  mode="pingpong", kind="bus",   phase=0.0),
    "bus_002":      dict(wp=BUS_002,       cruise=7.5,  mode="pingpong", kind="bus",   phase=1.7),
    "bus_046":      dict(wp=BUS_046,       cruise=8.5,  mode="pingpong", kind="bus",   phase=3.1),
    "metro_l1":     dict(wp=METRO_L1,      cruise=15.0, mode="pingpong", kind="train", phase=0.6),
    "metro_l6":     dict(wp=METRO_L6,      cruise=14.0, mode="loop",     kind="train", phase=2.4),
    "cercanias_c1": dict(wp=CERCANIAS_C1,  cruise=22.0, mode="pingpong", kind="train", phase=4.2),
    "drone_retiro": dict(wp=DRONE_RETIRO,  cruise=12.0, mode="loop",     kind="drone"),
    "drone_madrid_rio": dict(wp=DRONE_MADRID_RIO, cruise=13.0, mode="loop", kind="drone"),
}


def _seg_len_m(a, b):
    dn = (b[0] - a[0]) * M_PER_DEG_LAT
    de = (b[1] - a[1]) * _m_per_deg_lon(a[0])
    return math.hypot(dn, de)


def _point_at(wp, dist_m, mode):
    """Position along the polyline at arc length dist_m (metres)."""
    lengths = [_seg_len_m(wp[i], wp[i + 1]) for i in range(len(wp) - 1)]
    total = sum(lengths)
    if total == 0:
        return wp[0]

    if mode == "loop":
        d = dist_m % total
    else:  # pingpong
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
    wp, cruise, mode, kind = cfg["wp"], cfg["cruise"], cfg["mode"], cfg["kind"]
    rows: list[dict] = []
    prev = None
    for t in range(0, DURATION_S + 1, STEP_S):
        lat, lon = _point_at(wp, cruise * t, mode)

        if prev is None:
            speed, heading = cruise, 0.0
        else:
            dn = (lat - prev[0]) * M_PER_DEG_LAT
            de = (lon - prev[1]) * _m_per_deg_lon(lat)
            speed = math.hypot(dn, de) / STEP_S
            heading = (math.degrees(math.atan2(de, dn)) + 360.0) % 360.0

        row = {
            "timestamp": t,
            "latitude": round(lat, 6),
            "longitude": round(lon, 6),
            "speed": round(speed, 2),
            "heading": round(heading, 1),
        }
        if kind == "drone":
            row["altitude"] = round(80.0 + 25.0 * math.sin(t / 90.0), 1)
            row["battery"] = round(max(8.0, 100.0 - 2.4 * (t / 60.0)), 1)
        else:
            # occupancy swells at rush-hour-ish intervals
            base = 55.0 if kind == "train" else 40.0
            phase = cfg.get("phase", 0.0)
            row["occupancy"] = round(
                min(100.0, max(5.0, base + 35.0 * math.sin(t / 240.0 + phase))), 1
            )
        rows.append(row)
        prev = (lat, lon)
    return rows


def main() -> None:
    os.makedirs(OUT_DIR, exist_ok=True)
    for name, cfg in ROUTES.items():
        rows = build_trace(cfg)
        fields = list(rows[0].keys())
        path = os.path.join(OUT_DIR, f"{name}.csv")
        with open(path, "w", newline="") as fh:
            w = csv.DictWriter(fh, fieldnames=fields)
            w.writeheader()
            w.writerows(rows)
        print(f"  {name:20s} {cfg['kind']:6s} {len(rows)} rows  -> {path}")
    print(f"Done - {len(ROUTES)} Madrid routes in {OUT_DIR}/")


if __name__ == "__main__":
    main()
