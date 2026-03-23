"""
mobility_manager.py
-------------------
Manages mobility traces for all devices in a SimGrid simulation.

# Quick-start

## 1. Add ``mobility_dir`` property to your platform XML

    <zone id="root" routing="Full">
      <prop id="mobility_dir" value="platforms/coords/"/>
      ...
    </zone>

## 2. Create the manager *after* loading the platform

    import simgrid
    from enigma.mobility import MobilityManager

    e = simgrid.Engine(sys.argv)
    e.load_platform(platform_xml)

    mob = MobilityManager(e)               # reads property from XML
    # -- or --
    mob = MobilityManager(e, coords_dir="platforms/coords/")

## 3. Query positions inside actors

    pos = mob.position_at("edge_cluster_0_node_1", simgrid.Engine.clock)
    if pos:
        simgrid.this_actor.info(f"lat={pos.latitude:.6f} lon={pos.longitude:.6f}")

## 4. Start the periodic snapshot actor (optional)

    mob.start_periodic_actor(e, interval_s=0.5)

## 5. After e.run(), export

    mob.recorder.export_csv("output_snapshots.csv")
    mob.recorder.export_json("output_snapshots.json")
"""
from __future__ import annotations

import glob
import os
import queue
import threading
from typing import Dict, List, Optional

import simgrid

from .mobility_trace import MobilityTrace, MobilityPosition
from .mobility_recorder import MobilityRecorder


class MobilityManager:
    """
    Central manager: loads traces, interpolates positions, records snapshots.

    Parameters
    ----------
    engine : simgrid.Engine
        Running SimGrid engine (must have loaded a platform).
    coords_dir : str, optional
        Directory with ``<device_name>.csv`` files.  If omitted the manager
        reads the ``mobility_dir`` zone/host property from the platform.
    visualizer : MobilityVisualizer, optional
        If provided, position updates are forwarded there for live display.
    """

    def __init__(
        self,
        engine: simgrid.Engine,
        coords_dir: Optional[str] = None,
        visualizer=None,
        platform_xml: Optional[str] = None,
    ) -> None:
        self._engine     = engine
        self._traces:    Dict[str, MobilityTrace] = {}
        self._recorder   = MobilityRecorder()
        self._visualizer = visualizer
        self._lock       = threading.Lock()

        dir_path = coords_dir or self._detect_mobility_dir(engine, platform_xml)
        if not dir_path:
            print("[MobilityManager] No mobility_dir found – module inactive")
        else:
            self._load_directory(dir_path)

    # ------------------------------------------------------------------ #
    # Properties                                                            #
    # ------------------------------------------------------------------ #

    @property
    def recorder(self) -> MobilityRecorder:
        return self._recorder

    @property
    def trace_count(self) -> int:
        return len(self._traces)

    @property
    def device_names(self) -> List[str]:
        return list(self._traces.keys())

    # ------------------------------------------------------------------ #
    # Queries                                                               #
    # ------------------------------------------------------------------ #

    def has_trace(self, device_name: str) -> bool:
        return device_name in self._traces

    def position_at(self, device_name: str, sim_t: float) -> Optional[MobilityPosition]:
        """
        Return the interpolated position for *device_name* at simulation
        time *sim_t*, or ``None`` if no trace is loaded for that device.
        """
        trace = self._traces.get(device_name)
        if trace is None:
            return None
        return trace.position_at(sim_t)

    def get_trace(self, device_name: str) -> Optional[MobilityTrace]:
        return self._traces.get(device_name)

    # ------------------------------------------------------------------ #
    # Recording helpers (called from within SimGrid actors)                #
    # ------------------------------------------------------------------ #

    def record(self, device_name: str, sim_t: float) -> Optional[MobilityPosition]:
        """
        Compute and record position for one device.
        Returns the position (or None if no trace).
        Thread-safe.
        """
        pos = self.position_at(device_name, sim_t)
        if pos is None:
            return None
        with self._lock:
            self._recorder.add(device_name, pos)
            if self._visualizer is not None:
                self._visualizer.push(device_name, pos)
        return pos

    def record_all(self, sim_t: float) -> Dict[str, MobilityPosition]:
        """
        Compute and record positions for all loaded devices at *sim_t*.
        Returns a dict {device_name: MobilityPosition}.
        Thread-safe.
        """
        result: Dict[str, MobilityPosition] = {}
        with self._lock:
            for name, trace in self._traces.items():
                pos = trace.position_at(sim_t)
                self._recorder.add(name, pos)
                result[name] = pos
                if self._visualizer is not None:
                    self._visualizer.push(name, pos)
        return result

    # ------------------------------------------------------------------ #
    # Periodic actor                                                        #
    # ------------------------------------------------------------------ #

    def start_periodic_actor(
        self,
        engine: simgrid.Engine,
        interval_s: float = 1.0,
    ) -> None:
        """
        Deploy a SimGrid actor that calls ``record_all()`` every
        *interval_s* simulated seconds.  Must be called before ``e.run()``.
        The actor automatically stops once the simulation clock passes the
        latest timestamp in any loaded trace (+ one extra interval).
        """
        if not self._traces:
            return

        # Determine the latest timestamp across all traces
        t_end = max(t.t_max for t in self._traces.values()) + interval_s * 2

        host = engine.all_hosts[0]
        manager = self   # capture reference

        def _recorder_actor():
            simgrid.this_actor.info(
                f"[MobilityRecorder] started  interval={interval_s:.2f} s  "
                f"runs until t={t_end:.1f} s"
            )
            while simgrid.Engine.clock <= t_end:
                t = simgrid.Engine.clock
                manager.record_all(t)
                simgrid.this_actor.sleep_for(interval_s)
            simgrid.this_actor.info("[MobilityRecorder] finished")

        simgrid.Actor.create("mobility_recorder", host, _recorder_actor)

    # ------------------------------------------------------------------ #
    # Internal                                                              #
    # ------------------------------------------------------------------ #

    @staticmethod
    def _detect_mobility_dir(engine: simgrid.Engine,
                              platform_xml: Optional[str] = None) -> str:
        """Return the value of the 'mobility_dir' property from any zone/host.

        SimGrid's Python bindings expose ``host.get_property()`` but **zone**
        properties are not propagated down to hosts.  We therefore parse the
        XML file directly when the path is known.
        """
        # 1. Try host-level properties (works if prop is on a host, not a zone)
        for host in engine.all_hosts:
            try:
                val = host.get_property("mobility_dir")
                if val:
                    return val
            except Exception:
                pass

        # 2. Parse the XML directly – handles zone-level <prop> elements
        if platform_xml and os.path.isfile(platform_xml):
            try:
                import xml.etree.ElementTree as ET
                tree = ET.parse(platform_xml)
                for elem in tree.iter():
                    if elem.tag == "prop" and elem.get("id") == "mobility_dir":
                        return elem.get("value", "")
            except Exception:
                pass

        return ""

    def _load_directory(self, dir_path: str) -> None:
        if not os.path.isdir(dir_path):
            print(f"[MobilityManager] Directory not found: {dir_path}")
            return

        csv_files = glob.glob(os.path.join(dir_path, "*.csv"))
        if not csv_files:
            print(f"[MobilityManager] No CSV files found in {dir_path}")
            return

        loaded = 0
        for csv_path in sorted(csv_files):
            try:
                t = MobilityTrace(csv_path)
                self._traces[t.device_name] = t
                loaded += 1
            except Exception as ex:
                print(f"[MobilityManager] Warning: could not load {csv_path}: {ex}")

        print(
            f"[MobilityManager] Loaded {loaded} traces from '{dir_path}'"
        )
