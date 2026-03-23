"""
mobility_trace.py
-----------------
Load and linearly interpolate one device's GPS trace from a CSV file.

CSV format (header row mandatory):
    timestamp, latitude, longitude [, <any extra column>...]

Only ``timestamp``, ``latitude`` (aliases: ``lat``) and ``longitude``
(aliases: ``lon``, ``lng``) are required.  **All other columns – whatever
their name – are stored transparently in** ``MobilityPosition.extra`` and
are interpolated / exported automatically.  There is no assumed schema
beyond the three required fields.

Usage
-----
    from enigma.mobility import MobilityTrace

    trace = MobilityTrace("platforms/coords/device1.csv")
    pos   = trace.position_at(3.5)          # interpolated at t=3.5 s
    print(pos.latitude, pos.longitude)
    print(pos.extra)                        # {"altitude": ..., "speed": ..., ...}
"""
from __future__ import annotations

import csv
import math
import os
from dataclasses import dataclass, field
from typing import Dict, List, Optional


# Column aliases accepted for the three mandatory fields (checked lowercase)
_TS_ALIASES  = {"timestamp", "time", "t", "ts", "time_s", "sim_time"}
_LAT_ALIASES = {"latitude",  "lat",  "lat_deg"}
_LON_ALIASES = {"longitude", "lon",  "lng",  "lon_deg", "long"}


# --------------------------------------------------------------------------- #
# MobilityPosition                                                             #
# --------------------------------------------------------------------------- #

@dataclass
class MobilityPosition:
    """
    One position snapshot (one CSV row, possibly interpolated).

    Only ``timestamp``, ``latitude`` and ``longitude`` are first-class fields.
    Everything else from the CSV is stored in ``extra`` keyed by the original
    column name (lower-cased and stripped).
    """
    timestamp: float = 0.0          # simulation / wall-clock time (s)
    latitude:  float = 0.0          # decimal degrees, WGS-84
    longitude: float = 0.0          # decimal degrees, WGS-84
    extra:     Dict[str, float] = field(default_factory=dict)

    # Earth radius (metres, WGS-84 mean)
    _R = 6_371_000.0

    def distance_to(self, other: "MobilityPosition") -> float:
        """Haversine distance to *other* in metres."""
        phi1 = math.radians(self.latitude)
        phi2 = math.radians(other.latitude)
        dphi = math.radians(other.latitude  - self.latitude)
        dlam = math.radians(other.longitude - self.longitude)
        a = math.sin(dphi / 2) ** 2 + \
            math.cos(phi1) * math.cos(phi2) * math.sin(dlam / 2) ** 2
        return self._R * 2.0 * math.atan2(math.sqrt(a), math.sqrt(1.0 - a))

    def as_dict(self) -> dict:
        """Return all fields as a flat dict (timestamp + latitude + longitude + extra)."""
        return {
            "timestamp": self.timestamp,
            "latitude":  self.latitude,
            "longitude": self.longitude,
            **self.extra,
        }


# --------------------------------------------------------------------------- #
# MobilityTrace                                                                #
# --------------------------------------------------------------------------- #

class MobilityTrace:
    """
    Loads a device's CSV trace and provides linear interpolation.

    Parameters
    ----------
    csv_path : str
        Path to the CSV file.  The file stem is used as ``device_name``.
    """

    def __init__(self, csv_path: str) -> None:
        self._csv_path    = csv_path
        self._device_name = os.path.splitext(os.path.basename(csv_path))[0]
        self._waypoints:  List[MobilityPosition] = []
        self._load(csv_path)

    # ------------------------------------------------------------------ #
    # Properties                                                            #
    # ------------------------------------------------------------------ #

    @property
    def device_name(self) -> str:
        return self._device_name

    @property
    def csv_path(self) -> str:
        return self._csv_path

    @property
    def waypoints(self) -> List[MobilityPosition]:
        return list(self._waypoints)

    @property
    def size(self) -> int:
        return len(self._waypoints)

    @property
    def t_min(self) -> float:
        return self._waypoints[0].timestamp if self._waypoints else 0.0

    @property
    def t_max(self) -> float:
        return self._waypoints[-1].timestamp if self._waypoints else 0.0

    # ------------------------------------------------------------------ #
    # Interpolation                                                         #
    # ------------------------------------------------------------------ #

    def position_at(self, sim_t: float) -> MobilityPosition:
        """
        Linear interpolation at simulation time *sim_t*.

        - Before t_min: returns the first waypoint.
        - After  t_max: returns the last  waypoint.
        - Otherwise:    linearly interpolates between the two surrounding entries.
        """
        wps = self._waypoints
        if not wps:
            return MobilityPosition(timestamp=sim_t)
        if sim_t <= wps[0].timestamp:
            p = wps[0]
            return MobilityPosition(sim_t, p.latitude, p.longitude, dict(p.extra))
        if sim_t >= wps[-1].timestamp:
            p = wps[-1]
            return MobilityPosition(sim_t, p.latitude, p.longitude, dict(p.extra))

        # Binary search for the first waypoint with timestamp > sim_t
        lo, hi = 0, len(wps) - 1
        while lo + 1 < hi:
            mid = (lo + hi) // 2
            if wps[mid].timestamp <= sim_t:
                lo = mid
            else:
                hi = mid

        a, b = wps[lo], wps[hi]
        frac = (sim_t - a.timestamp) / (b.timestamp - a.timestamp)
        return self._lerp(a, b, sim_t, frac)

    # ------------------------------------------------------------------ #
    # Internal                                                              #
    # ------------------------------------------------------------------ #

    def _load(self, csv_path: str) -> None:
        with open(csv_path, newline="") as f:
            reader = csv.DictReader(f)
            if reader.fieldnames is None:
                raise ValueError(f"Empty CSV file: {csv_path}")

            # Map each raw header to its canonical role: "timestamp", "latitude",
            # "longitude", or its own lower-cased name (→ extra).
            ts_col  = lat_col = lon_col = None
            extra_cols: List[str] = []  # raw header names that become extras

            for raw in reader.fieldnames:
                norm = raw.strip().lower()
                if norm in _TS_ALIASES and ts_col is None:
                    ts_col = raw
                elif norm in _LAT_ALIASES and lat_col is None:
                    lat_col = raw
                elif norm in _LON_ALIASES and lon_col is None:
                    lon_col = raw
                else:
                    extra_cols.append(raw)  # keep original casing for readability

            if ts_col is None or lat_col is None or lon_col is None:
                missing = []
                if ts_col  is None: missing.append("timestamp")
                if lat_col is None: missing.append("latitude")
                if lon_col is None: missing.append("longitude")
                raise ValueError(
                    f"CSV '{csv_path}' is missing required column(s): {missing}. "
                    f"Got: {list(reader.fieldnames)}"
                )

            # Canonicalise extra column names (lower-case, stripped) for the extra dict
            canonical_extras = [c.strip().lower() for c in extra_cols]

            for raw in reader:
                row = {k.strip(): v.strip() for k, v in raw.items() if v is not None}
                try:
                    p = MobilityPosition(
                        timestamp = float(row[ts_col]),
                        latitude  = float(row[lat_col]),
                        longitude = float(row[lon_col]),
                        extra     = {
                            canon: float(row.get(orig, "") or 0)
                            for orig, canon in zip(extra_cols, canonical_extras)
                            if row.get(orig, "").strip() != ""
                        },
                    )
                    self._waypoints.append(p)
                except (ValueError, KeyError):
                    continue  # skip malformed rows

        if not self._waypoints:
            raise ValueError(f"No valid rows in {csv_path}")

        # Ensure sorted by timestamp
        self._waypoints.sort(key=lambda p: p.timestamp)

    @classmethod
    def _lerp(cls, a: MobilityPosition, b: MobilityPosition,
              sim_t: float, frac: float) -> MobilityPosition:
        def ld(va, vb): return va + (vb - va) * frac
        # Interpolate all extra keys from both waypoints
        all_keys = set(a.extra) | set(b.extra)
        extra = {k: ld(a.extra.get(k, 0.0), b.extra.get(k, 0.0)) for k in all_keys}
        return MobilityPosition(
            timestamp = sim_t,
            latitude  = ld(a.latitude,  b.latitude),
            longitude = ld(a.longitude, b.longitude),
            extra     = extra,
        )
