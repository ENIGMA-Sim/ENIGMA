# ENIGMA Mobility Module — Python

Python implementation of the ENIGMA mobility module.  Wraps SimGrid's Python
bindings to load GPS traces, interpolate device positions, record snapshots
during the simulation, and generate interactive **folium** maps.

## Files

| File | Purpose |
|------|---------|
| `mobility_trace.py` | CSV loader + linear interpolation for one device |
| `mobility_manager.py` | Central manager: load traces, record, integrate with SimGrid |
| `mobility_recorder.py` | Thread-safe snapshot collection + CSV/JSON/GeoJSON export |
| `mobility_visualizer.py` | Folium interactive map (post-sim and live during sim) |

---

## Requirements

```bash
pip install folium          # for interactive map generation
# SimGrid Python bindings are installed with libsimgrid-dev
```

---

## Quick-start

### 1 — CSV trace format

One file per device; filename stem must match the SimGrid host name exactly.

```
platforms/coords/
├── edge_0.csv
├── edge_1.csv
└── ...
```

```csv
timestamp,latitude,longitude,speed,heading
0,48.8572,2.3540,5.0,90.0
10,48.8575,2.3543,5.1,91.2
...
```

Only `timestamp`, `latitude` / `lat`, and `longitude` / `lon` / `lng` are
required.  Any extra columns (whatever names, however many) are loaded
automatically, interpolated, and shown in map popups.  Non-numeric values
(e.g. `stop_name`) are ignored per field — they do not cause the trace to fail.

### 2 — Declare the coords directory in the platform XML

```xml
<zone id="root" routing="Full">
  <prop id="mobility_dir" value="platforms/coords/"/>
  <!-- ... hosts, links ... -->
</zone>
```

The path is relative to the **working directory** where you run the script.

### 3 — Minimal simulation

```python
import sys
import simgrid
from enigma.mobility import MobilityManager, MobilityVisualizer

def mobile_actor(mob: MobilityManager, host_name: str) -> None:
    for _ in range(20):
        t   = simgrid.Engine.clock
        pos = mob.record(host_name, t)          # record + return interpolated position
        if pos:
            spd = pos.extra.get("speed", 0.0)
            simgrid.this_actor.info(
                f"t={t:.1f}  lat={pos.latitude:.6f}  lon={pos.longitude:.6f}  spd={spd:.1f}")
        simgrid.this_actor.sleep_for(1.0)

def main() -> None:
    e = simgrid.Engine(sys.argv)
    e.load_platform(sys.argv[1])                # XML must include mobility_dir prop

    mob = MobilityManager(e, platform_xml=sys.argv[1])  # reads mobility_dir from XML
    # mob = MobilityManager(e, coords_dir="platforms/coords/")  # or explicit path

    print(f"Traces loaded: {mob.trace_count}")

    mob.start_periodic_actor(e, interval_s=0.5) # record every 0.5 sim-seconds

    for host in e.all_hosts:
        if mob.has_trace(host.name):
            simgrid.Actor.create("worker", host,
                                 lambda h=host.name: mobile_actor(mob, h))

    e.run()

    mob.recorder.print_stats()
    mob.recorder.export_csv("snapshots.csv")
    mob.recorder.export_json("snapshots.json")
    mob.recorder.export_geojson("trajectories.geojson")

    # Interactive map
    MobilityVisualizer.save_interactive_map(mob.recorder, "map.html")
    # Offline (self-contained, no HTTP server needed):
    # MobilityVisualizer.save_interactive_map(mob.recorder, "map.html", offline=True)

main()
```

### 4 — Run the demo

```bash
# Fastest (no live window, no browser popup):
python3 src/python/tests/mobility_test.py platforms/coords2_platform.xml \
    --no-live --no-replay

# With custom output directory:
python3 src/python/tests/mobility_test.py platforms/coords2_platform.xml \
    --output /home/user/results --no-live --no-replay

# Offline map (opens directly as file://, no HTTP server):
python3 src/python/tests/mobility_test.py platforms/coords2_platform.xml \
    --no-live --offline

# With live map in browser during simulation:
python3 src/python/tests/mobility_test.py platforms/coords2_platform.xml

# All flags:
python3 src/python/tests/mobility_test.py platforms/coords2_platform.xml \
    --output /tmp/out --interval 5.0 --no-live --no-replay --offline
```

---

## API reference

### `MobilityTrace`

```python
from enigma.mobility import MobilityTrace

trace = MobilityTrace("coords/edge_0.csv")

trace.device_name          # "edge_0"
trace.size                 # number of waypoints
trace.t_min; trace.t_max  # time range (seconds)

pos = trace.position_at(3.7)   # MobilityPosition, linearly interpolated
```

### `MobilityPosition`

```python
pos.timestamp    # float – simulation seconds
pos.latitude     # float – decimal degrees
pos.longitude    # float – decimal degrees
pos.extra        # dict[str, float] – all other CSV columns

pos.distance_to(other)   # Haversine distance in metres
pos.as_dict()            # {"timestamp": ..., "latitude": ..., "longitude": ..., ...extra}
```

### `MobilityManager`

```python
from enigma.mobility import MobilityManager

# Construction
mob = MobilityManager(engine)                              # reads mobility_dir from XML zone prop
mob = MobilityManager(engine, platform_xml="my.xml")       # explicit XML (needed for zone props)
mob = MobilityManager(engine, coords_dir="platforms/coords/")  # or explicit directory

# Query
mob.trace_count                          # int
mob.has_trace("edge_0")                  # bool
pos = mob.position_at("edge_0", t)       # MobilityPosition | None

# Record position (returns the interpolated position, or None)
pos = mob.record("edge_0", t)

# Periodic actor (deploy before e.run())
mob.start_periodic_actor(engine, interval_s=0.5)

# Access the recorder
mob.recorder                             # MobilityRecorder instance
```

### `MobilityRecorder`

```python
from enigma.mobility import MobilityRecorder

rec = MobilityRecorder()
rec.add("edge_0", pos)                   # append a snapshot

rec.count                                # total snapshots
rec.devices()                            # list of device names
rec.records_for("edge_0")               # list[MobilityPosition]
rec.by_device()                          # dict[str, list[MobilityPosition]]
rec.infer_interval_s()                   # auto-detect recording interval

rec.print_stats()                        # summary table to stdout

rec.export_csv("out.csv")
rec.export_json("out.json")
rec.export_geojson("out.geojson")
```

### `MobilityVisualizer`

#### Post-simulation (static method)

```python
from enigma.mobility import MobilityVisualizer

# Online (default) — opens via Playwright as file:// directly (no HTTP server needed)
MobilityVisualizer.save_interactive_map(recorder, "map.html")

# Offline — embeds all JS/CSS, opens as file:// directly (~6.5 MB)
MobilityVisualizer.save_interactive_map(recorder, "map.html", offline=True)

# With explicit time-slider step and title
MobilityVisualizer.save_interactive_map(
    recorder, "map.html",
    title="Bus routes – Madrid",
    interval_s=10.0,
    offline=True,
)
```

#### Live (during simulation)

A Playwright/Chromium window opens automatically and refreshes while the simulation runs.  When the simulation ends the final map is navigated to inside the same window; calling `wait_for_close()` blocks the process until the user closes the browser.

```python
viz = MobilityVisualizer(
    title             = "My Sim – Live",
    live_path         = "/tmp/out/mobility_live.html",  # where the live HTML is written
    update_interval_s = 3.0,                            # rebuilt every 3 real seconds
    offline           = False,                          # use True to embed CDN inline
)
viz.start()   # opens Playwright/Chromium automatically

mob = MobilityManager(engine, coords_dir="platforms/coords/", visualizer=viz)
mob.start_periodic_actor(engine, interval_s=0.5)
engine.run()

viz.stop()                                            # keeps browser open after sim
viz.show_final("/tmp/out/replay.html")                # navigate to final full map
viz.wait_for_close()                                  # exits when user closes browser
```

---

## Standalone viewer (any snapshot file)

Works with output from **both** C++ and Python simulations:

```bash
# Basic (online mode)
python3 src/python/tools/mobility_viewer.py snapshots.json

# Offline, custom output path
python3 src/python/tools/mobility_viewer.py snapshots.json \
    --offline --save my_map.html

# Statistics only (no map)
python3 src/python/tools/mobility_viewer.py snapshots.csv --stats-only

# Filter specific devices
python3 src/python/tools/mobility_viewer.py snapshots.json \
    --devices edge_0 edge_1 --offline
```

### Online vs offline mode

Because the visualizer always uses Playwright/Chromium, both modes open as `file://` directly — no HTTP server is needed.

| Mode | CDN resources | Internet needed to view | HTML size |
|------|:---:|:---:|:---:|
| `--online` (default) | fetched at view time | ✅ CDN + map tiles | ~1.5 MB |
| `--offline` | embedded inline | map tiles only | ~6.5 MB |

---

## Map features

- **Trajectory lines** for every device on OpenStreetMap
- **Time slider** — play/pause animation; step matches recording interval
- **Click any dot** — popup with all recorded stats for that snapshot
- **Layer toggle** and device colour legend
- Online: ~1.5 MB, CDN resources loaded at view time
- Offline (`--offline`): ~6.5 MB, fully self-contained `file://`
