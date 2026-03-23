"""
mobility_recorder.py
--------------------
Collects MobilityPosition snapshots during a simulation and exports them
to CSV / JSON for post-simulation analysis and replay.

Usage
-----
    recorder = MobilityRecorder()
    recorder.add("device1", pos)          # call from actors

    # after e.run()
    recorder.export_csv("mobility.csv")
    recorder.export_json("mobility.json")
    recorder.export_geojson("mobility.geojson")
"""
from __future__ import annotations

import json
import csv as _csv_mod
import os
from typing import Dict, List, Tuple

from .mobility_trace import MobilityPosition


class MobilityRecorder:
    """
    Thread-safe collection of position snapshots.

    Each snapshot is a (device_name, MobilityPosition) pair stored in
    insertion order.  Designed to be called concurrently from SimGrid actors.
    """

    def __init__(self) -> None:
        self._records: List[Tuple[str, MobilityPosition]] = []

    # ------------------------------------------------------------------ #
    # Recording                                                             #
    # ------------------------------------------------------------------ #

    def add(self, device_name: str, position: MobilityPosition) -> None:
        """Append a snapshot (NOT thread-safe on its own; lock from caller)."""
        self._records.append((device_name, position))

    def clear(self) -> None:
        self._records.clear()

    # ------------------------------------------------------------------ #
    # Introspection                                                          #
    # ------------------------------------------------------------------ #

    @property
    def count(self) -> int:
        return len(self._records)

    def records(self) -> List[Tuple[str, MobilityPosition]]:
        return list(self._records)

    def devices(self) -> List[str]:
        seen: List[str] = []
        for name, _ in self._records:
            if name not in seen:
                seen.append(name)
        return seen

    def records_for(self, device_name: str) -> List[MobilityPosition]:
        return [pos for name, pos in self._records if name == device_name]

    def by_device(self) -> Dict[str, List[MobilityPosition]]:
        """Return a dict mapping device_name → list of positions (in time order)."""
        out: Dict[str, List[MobilityPosition]] = {}
        for name, pos in self._records:
            out.setdefault(name, []).append(pos)
        return out

    def infer_interval_s(self) -> float:
        """
        Estimate the snapshot recording interval (seconds) from the data.

        Takes the median gap between consecutive timestamps of the first
        device that has at least 2 snapshots.  Falls back to 1.0 if the
        recorder is empty or contains only a single snapshot per device.
        """
        for name in self.devices():
            pts = self.records_for(name)
            if len(pts) >= 2:
                gaps = [
                    pts[i + 1].timestamp - pts[i].timestamp
                    for i in range(len(pts) - 1)
                    if pts[i + 1].timestamp > pts[i].timestamp
                ]
                if gaps:
                    gaps.sort()
                    return gaps[len(gaps) // 2]  # median
        return 1.0

    # ------------------------------------------------------------------ #
    # Export: CSV                                                           #
    # ------------------------------------------------------------------ #

    def export_csv(self, filepath: str) -> None:
        """
        Write all snapshots to a CSV file.
        Columns: device, timestamp, latitude, longitude, <all extra columns found>
        The set of extra columns is discovered dynamically from the recorded data.
        """
        os.makedirs(os.path.dirname(filepath) if os.path.dirname(filepath) else ".", exist_ok=True)

        # Build ordered list of all column names seen across all snapshots
        all_keys: List[str] = ["device"]
        seen_keys: List[str] = []
        for _, pos in self._records:
            for k in pos.as_dict():
                if k not in seen_keys:
                    seen_keys.append(k)
        all_keys.extend(seen_keys)

        with open(filepath, "w", newline="") as f:
            writer = _csv_mod.DictWriter(f, fieldnames=all_keys, extrasaction="ignore")
            writer.writeheader()
            for name, pos in self._records:
                row = {"device": name, **pos.as_dict()}
                writer.writerow(row)

        print(f"[MobilityRecorder] Exported {len(self._records)} snapshots → {filepath}")

    # ------------------------------------------------------------------ #
    # Export: JSON                                                          #
    # ------------------------------------------------------------------ #

    def export_json(self, filepath: str, indent: int = 2) -> None:
        """Write snapshots to JSON (array of objects)."""
        os.makedirs(os.path.dirname(filepath) if os.path.dirname(filepath) else ".", exist_ok=True)
        data = [{"device": name, **pos.as_dict()} for name, pos in self._records]
        with open(filepath, "w") as f:
            json.dump(data, f, indent=indent)
        print(f"[MobilityRecorder] Exported {len(self._records)} snapshots → {filepath}")

    # ------------------------------------------------------------------ #
    # Export: GeoJSON                                                       #
    # ------------------------------------------------------------------ #

    def export_geojson(self, filepath: str) -> None:
        """
        Write snapshots as a GeoJSON FeatureCollection.

        Each device becomes a LineString feature (its trajectory), plus
        individual Point features for each snapshot.
        """
        os.makedirs(os.path.dirname(filepath) if os.path.dirname(filepath) else ".", exist_ok=True)

        by_dev = self.by_device()
        features = []

        for device, positions in by_dev.items():
            # Use [lon, lat] or [lon, lat, alt] when altitude is available
            def _coord(p):
                if "altitude" in p.extra:
                    return [p.longitude, p.latitude, p.extra["altitude"]]
                return [p.longitude, p.latitude]

            coords = [_coord(p) for p in positions]
            # LineString trajectory
            features.append({
                "type": "Feature",
                "properties": {
                    "device": device,
                    "type": "trajectory",
                    "n_points": len(positions),
                },
                "geometry": {
                    "type": "LineString",
                    "coordinates": coords,
                },
            })
            # First and last position as Points (include all extra stats)
            for tag, pos in [("start", positions[0]), ("end", positions[-1])]:
                features.append({
                    "type": "Feature",
                    "properties": {
                        "device": device,
                        "type": tag,
                        **pos.as_dict(),
                    },
                    "geometry": {
                        "type": "Point",
                        "coordinates": _coord(pos),
                    },
                })

        geojson = {"type": "FeatureCollection", "features": features}
        with open(filepath, "w") as f:
            json.dump(geojson, f, indent=2)
        print(f"[MobilityRecorder] Exported GeoJSON ({len(by_dev)} devices) → {filepath}")

    # ------------------------------------------------------------------ #
    # Quick statistics                                                      #
    # ------------------------------------------------------------------ #

    def print_stats(self) -> None:
        by_dev = self.by_device()
        print(f"\n{'='*60}")
        print(f"  MobilityRecorder statistics  ({len(self._records)} snapshots total)")
        print(f"{'='*60}")
        for device, positions in sorted(by_dev.items()):
            ts = [p.timestamp for p in positions]
            total_dist = sum(
                positions[i].distance_to(positions[i + 1])
                for i in range(len(positions) - 1)
            )
            # Compute averages for all extra columns that appear in this device
            extra_avg: Dict[str, float] = {}
            for p in positions:
                for k, v in p.extra.items():
                    extra_avg.setdefault(k, 0.0)
                    extra_avg[k] += v
            extra_str = "  ".join(
                f"{k}={extra_avg[k]/len(positions):.2f}avg"
                for k in sorted(extra_avg)
            )
            print(
                f"  {device:<35}  "
                f"t=[{min(ts):.1f}–{max(ts):.1f}]s  "
                f"n={len(positions):4d}  "
                f"dist={total_dist/1000:.3f} km"
                + (f"  {extra_str}" if extra_str else "")
            )
        print()
