"""
mobility_test.py
----------------
ENIGMA mobility module demonstration using SimGrid Python bindings.

Shows:
  - Loading device traces from a coords directory (specified via XML property
    OR -coords-dir argument)
  - Querying interpolated positions inside SimGrid actors
  - Live window opened during simulation
  - Recording all snapshots to CSV / JSON / GeoJSON after simulation
  - Animated replay and static map generation

Usage
-----
    python3 tests/mobility_test.py <platform.xml> [options]

    Options:
      --coords-dir <dir>   Override the mobility_dir XML property
      --output <prefix>    Output file prefix (default: mobility_output/)
      --interval <secs>    Snapshot interval in sim-seconds (default: 0.5)
      --no-live            Disable live window (record + replay only)
      --fps <n>            Replay animation fps (default: 15)
      --save-replay        Save replay to mobility_output/replay.gif
"""
from __future__ import annotations

import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.join(_HERE, "..")
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

import signal
import simgrid

from enigma.mobility import (
    MobilityManager,
    MobilityVisualizer,
)


# ─────────────────────────────────────────────────────────────────────────── #
# Actors                                                                       #
# ─────────────────────────────────────────────────────────────────────────── #

class MobileActor:
    """
    A demo SimGrid actor that:
      - Periodically queries its own GPS position from MobilityManager
      - Does a small computation to advance simulated time
      - Logs its coordinates
    """

    def __init__(
        self,
        host_name: str,
        mob: MobilityManager,
        interval_s: float = 1.0,
        iterations: int   = 10,
    ) -> None:
        self._host_name  = host_name
        self._mob        = mob
        self._interval   = interval_s
        self._iterations = iterations

    def __call__(self) -> None:
        simgrid.this_actor.info(
            f"[{self._host_name}] Mobile actor started"
        )
        for i in range(self._iterations):
            t   = simgrid.Engine.clock
            pos = self._mob.record(self._host_name, t)

            if pos:
                spd = pos.extra.get("speed",   0.0)
                hdg = pos.extra.get("heading", 0.0)
                simgrid.this_actor.info(
                    f"[{self._host_name}] t={t:.2f}s  "
                    f"lat={pos.latitude:.6f}  lon={pos.longitude:.6f}  "
                    f"spd={spd:.1f} m/s  hdg={hdg:.0f}°"
                )
            else:
                simgrid.this_actor.info(
                    f"[{self._host_name}] t={t:.2f}s  (no mobility trace)"
                )

            # Light local computation to advance simulated time
            simgrid.this_actor.execute(2e8)
            simgrid.this_actor.sleep_for(self._interval)

        simgrid.this_actor.info(f"[{self._host_name}] Actor finished")


# ─────────────────────────────────────────────────────────────────────────── #
# main                                                                         #
# ─────────────────────────────────────────────────────────────────────────── #

def main() -> None:
    # ---- argument parsing ------------------------------------------------- #
    e = simgrid.Engine(sys.argv)

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <platform.xml> [--coords-dir <dir>] "
              "[--output <prefix>] [--interval <s>] [--no-live] "
              "[--no-replay] [--fps <n>] [--save-replay] [--online|--offline]", file=sys.stderr)
        sys.exit(1)

    platform_xml  = sys.argv[1]
    coords_dir    = None
    # Default output next to this script, not the working directory
    output_dir    = os.path.join(os.path.dirname(os.path.abspath(__file__)), "mobility_output")
    interval_s    = 0.5
    live_viz      = True
    fps           = 15.0
    save_replay   = False
    show_replay   = True
    offline_mode  = False

    i = 2
    while i < len(sys.argv):
        arg = sys.argv[i]
        if arg == "--coords-dir" and i + 1 < len(sys.argv):
            coords_dir = sys.argv[i + 1]; i += 2
        elif arg == "--output" and i + 1 < len(sys.argv):
            output_dir = sys.argv[i + 1]; i += 2
        elif arg == "--interval" and i + 1 < len(sys.argv):
            interval_s = float(sys.argv[i + 1]); i += 2
        elif arg == "--fps" and i + 1 < len(sys.argv):
            fps = float(sys.argv[i + 1]); i += 2
        elif arg == "--no-live":
            live_viz = False; i += 1
        elif arg == "--save-replay":
            save_replay = True; i += 1
        elif arg == "--no-replay":
            show_replay = False; i += 1
        elif arg == "--offline":
            offline_mode = True; i += 1
        elif arg == "--online":
            offline_mode = False; i += 1
        else:
            i += 1

    os.makedirs(output_dir, exist_ok=True)

    # ---- load platform ---------------------------------------------------- #
    e.load_platform(platform_xml)
    hosts = e.all_hosts

    simgrid.this_actor.info("=== ENIGMA Mobility Test Application ===")
    simgrid.this_actor.info(f"Platform  : {platform_xml}")
    simgrid.this_actor.info(f"Hosts     : {len(hosts)}")
    simgrid.this_actor.info(f"Output    : {output_dir}/")

    # ---- visualizer ------------------------------------------------------- #
    viz = None
    if live_viz:
        viz = MobilityVisualizer(
            title="ENIGMA – Device Mobility (live)",
            live_path=os.path.join(output_dir, "mobility_live.html"),
        )
        viz.start()
        simgrid.this_actor.info("Live visualization window opened (Playwright)")

    # ---- mobility manager ------------------------------------------------- #
    mob = MobilityManager(e, coords_dir=coords_dir, visualizer=viz,
                          platform_xml=platform_xml)
    simgrid.this_actor.info(f"Traces loaded : {mob.trace_count}")

    # ---- periodic snapshot actor ------------------------------------------ #
    mob.start_periodic_actor(e, interval_s=interval_s)

    # ---- deploy mobile actors --------------------------------------------- #
    deployed = 0
    for host in hosts:
        if mob.has_trace(host.name):
            simgrid.Actor.create(
                "mobile_actor", host,
                MobileActor(host.name, mob, interval_s=1.5,
                            iterations=mob.get_trace(host.name).size),
            )
            deployed += 1
            simgrid.this_actor.info(f"Deployed mobile actor on '{host.name}'")

    if deployed == 0:
        simgrid.this_actor.info(
            "No matching traces found for any host – "
            "check that CSV filenames match host names."
        )

    # ---- run simulation --------------------------------------------------- #
    try:
        e.run()
    except KeyboardInterrupt:
        print("\n[mobility_test] Simulation interrupted (Ctrl+C).")
        if viz:
            viz._running = False

    # SimGrid installs a persistent C-level SIGINT handler that stays active
    # even after e.run() returns.  If SIGINT arrives later (e.g. during the
    # input() prompt) SimGrid's handler fires and tries to decref Python objects
    # without the GIL → pybind11 SIGABRT (exit 134).
    # Restore Python's default SIGINT handler so Ctrl+C raises KeyboardInterrupt
    # normally from this point on.
    signal.signal(signal.SIGINT, signal.default_int_handler)

    simgrid.this_actor.info(
        f"=== Simulation completed – t={simgrid.Engine.clock:.3f} s ==="
    )
    simgrid.this_actor.info(
        f"Recorded {mob.recorder.count} snapshots  "
        f"for {len(mob.recorder.devices())} devices"
    )

    mob.recorder.print_stats()

    # ---- stop live window ------------------------------------------------- #
    if viz:
        viz.stop()

    # ---- export recordings ------------------------------------------------ #
    csv_path     = os.path.join(output_dir, "snapshots.csv")
    json_path    = os.path.join(output_dir, "snapshots.json")
    geojson_path = os.path.join(output_dir, "trajectories.geojson")
    map_path     = os.path.join(output_dir, "trajectory_map.html")

    mob.recorder.export_csv(csv_path)
    mob.recorder.export_json(json_path)
    mob.recorder.export_geojson(geojson_path)

    # ---- interactive map -------------------------------------------------- #
    MobilityVisualizer.save_interactive_map(
        mob.recorder, map_path,
        title="ENIGMA – Device Mobility",
        interval_s=interval_s,
        offline=offline_mode,
    )

    simgrid.this_actor.info("Outputs:")
    simgrid.this_actor.info(f"  {csv_path}")
    simgrid.this_actor.info(f"  {json_path}")
    simgrid.this_actor.info(f"  {geojson_path}")
    simgrid.this_actor.info(f"  {map_path}")

    # ---- interactive replay ----------------------------------------------- #
    if show_replay and viz is not None:
        simgrid.this_actor.info("Loading final map into Playwright browser…")
        viz.show_final(map_path)
        # Blocks until the user closes the browser window (or Ctrl+C).
        viz.wait_for_close()
    else:
        if viz is not None:
            viz.close_browser()
        simgrid.this_actor.info(f"Map saved → open manually: {map_path}")


if __name__ == "__main__":
    main()
