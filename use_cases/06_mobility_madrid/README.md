# Use Case 6 — Madrid Public-Transport Mobility

Demonstrates ENIGMA's **mobility module**: attaching real GPS routes to
SimGrid hosts, interpolating each device's position while the simulation runs,
recording periodic snapshots and exporting them for an interactive map.

## Scenario

Vehicles moving along **real Madrid routes**, reporting to two edge nodes
(Atocha, Nuevos Ministerios) that feed a fog control room and the cloud.

| Host | Line / route | Motion |
|------|--------------|--------|
| `edge_atocha`, `edge_nuevos_ministerios` | edge nodes at transport hubs | fixed |
| `fog_emt_control` | EMT / Metro control-room fog server | fixed |
| `cloud_dc` | cloud data centre | fixed |
| `bus_27` | EMT 27 — Pza. Castilla ↔ Embajadores (Castellana / Prado) | ping-pong |
| `bus_002` | EMT 2 — Manuel Becerra ↔ Argüelles (Alcalá / Gran Vía) | ping-pong |
| `bus_046` | EMT 46 — Atocha ↔ Moncloa (Prado / Gran Vía / Princesa) | ping-pong |
| `metro_l1` | Metro L1 — Chamartín → Pacífico | ping-pong |
| `metro_l6` | Metro L6 (Circular) — full ring | loop |
| `cercanias_c1` | Cercanías C-1 — Príncipe Pío → Aeropuerto T4 | ping-pong |
| `drone_retiro` | inspection drone — Retiro / museum-mile loop | loop |
| `drone_madrid_rio` | inspection drone — Manzanares / Madrid Río loop | loop |

Coordinates trace the real streets / rail alignments of each line (see the
waypoint tables in [`generate_traces.py`](generate_traces.py)). Mobile hosts
reach the nearest edge over per-device cellular links.

## Files

| File | Purpose |
|------|---------|
| [`madrid_transport_platform.xml`](madrid_transport_platform.xml) | Platform. Carries `<prop id="mobility_dir" .../>` |
| [`generate_traces.py`](generate_traces.py) | Regenerates `coords/*.csv` from the route tables (stdlib only) |
| `coords/*.csv` | One GPS trace per mobile host |

CSV columns:

```
buses / trains :  timestamp,latitude,longitude,speed,heading,occupancy
drones         :  timestamp,latitude,longitude,speed,heading,altitude,battery
```

The filename stem **must** match the SimGrid host id. Only `timestamp`,
`latitude`, `longitude` are mandatory; the rest are extra columns —
automatically interpolated and shown in the map popups.

## Run (C++ pipeline)

```bash
./use_cases/06_mobility_madrid/run.sh
```

Regenerates the traces, runs `mobility_test_app`, and writes:

```
/tmp/enigma_uc6_snapshots.json   recorded positions (+ interpolated extras)
/tmp/enigma_uc6_snapshots.csv    same data as CSV
/tmp/enigma_uc6_raw_traces.json  the full input waypoints per device
```

Override the location with `OUT_PREFIX=/my/path ./run.sh`.

## Visualise

```bash
# needs: pip install folium playwright && playwright install chromium
python3 src/python/tools/mobility_viewer.py /tmp/enigma_uc6_snapshots.json --offline
```

The map has one trajectory line per line/route, a **time slider** (step =
recording interval) to animate positions, and clickable dots showing every
recorded stat (speed / heading / occupancy / altitude / battery) at that
timestamp.

## Run (Python pipeline, live map)

```bash
python3 src/python/tests/mobility_test.py \
    use_cases/06_mobility_madrid/madrid_transport_platform.xml \
    --coords-dir use_cases/06_mobility_madrid/coords/

# headless (no browser), just export files + static map:
python3 src/python/tests/mobility_test.py \
    use_cases/06_mobility_madrid/madrid_transport_platform.xml \
    --coords-dir use_cases/06_mobility_madrid/coords/ \
    --no-live --no-replay --offline
```

## How it works

```cpp
MobilityManager mob(e);              // reads mobility_dir from the XML
mob.start_periodic_actor(e, 0.5);    // snapshot every device every 0.5 sim-s

auto pos = mob.position_at("metro_l6", sg4::Engine::get_clock());
// pos->latitude / pos->longitude, pos->extra["occupancy"], ...

mob.export_json("snapshots.json");   // after e.run()
```

## Try next

- Add or edit a line: append a waypoint list + a `ROUTES` entry in
  `generate_traces.py` and add a matching `<host>` in the platform XML.
- Add a real actor that reacts to position — e.g. hand a bus off from
  `edge_atocha` to `edge_nuevos_ministerios` when it crosses Cibeles.
- Drop your own AVL / GTFS-derived CSVs into `coords/` (one per host) to replay
  recorded fleet data.
