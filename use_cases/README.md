# ENIGMA — Use Cases

Ready-to-run scenarios that show how to install, configure and drive the
**ENIGMA** simulator. Each folder is self-contained: it generates (or ships)
its own platform, then runs a simulation and prints the results.

| # | Folder | What it demonstrates | Binary |
|---|--------|----------------------|--------|
| 1 | [`01_edge_computing/`](01_edge_computing/) | Local processing on edge devices + gateway aggregation | `edge_computing_app` |
| 2 | [`02_fog_analytics/`](02_fog_analytics/) | Streaming samples from edge to fog, real-time + aggregate analytics | `fog_analytics_app` |
| 3 | [`03_hybrid_edge_fog_cloud/`](03_hybrid_edge_fog_cloud/) | Three-tier Edge → Fog → Cloud data pipeline | `hybrid_cloud_app` |
| 4 | [`04_data_offloading/`](04_data_offloading/) | Run-time offloading decisions (local / fog / cloud) | `data_offloading_app` |
| 5 | [`05_mqtt_iot/`](05_mqtt_iot/) | MQTT publish/subscribe with the `enigma_mqtt` module | `mqtt_edge_app` |
| 6 | [`06_mobility_madrid/`](06_mobility_madrid/) | **GPS mobility**: Madrid buses / Metro / Cercanías trains + drones on real routes, position interpolation, snapshots + interactive map | `mobility_test_app` |
| 7 | [`07_mobility_france_trains/`](07_mobility_france_trains/) | **GPS mobility, step by step**: 5 TGV along real SNCF LGV corridors across France — built up in 4 stages | `mobility_test_app` |

## Requirements

- SimGrid **4.1+** (`sudo apt install simgrid libsimgrid-dev`, or build in `/opt/simgrid-4.1`)
- CMake 3.10+, a C++17 compiler
- Use cases 6–7 (interactive map only): `pip install folium playwright && playwright install chromium`

## How to run

```bash
# from anywhere inside the repo
./use_cases/01_edge_computing/run.sh
./use_cases/06_mobility_madrid/run.sh
```

Every `run.sh`:

1. sources [`common.sh`](common.sh), which auto-detects SimGrid and exports
   `LD_LIBRARY_PATH`;
2. calls `ensure_built <targets>` — configures + compiles **only** what is
   missing (first run builds the project into `build/`);
3. generates the platform XML with `platform_generator`;
4. runs the simulation from the repository root.

Nothing is installed system-wide and no file outside `build/`, `platforms/`
and `/tmp` is modified.

## Environment overrides

| Variable | Effect |
|----------|--------|
| `SIMGRID_ROOT` | Point `common.sh` at a specific SimGrid prefix |
| `OUT_PREFIX` | (use cases 6–7) output path prefix for the snapshot files |
