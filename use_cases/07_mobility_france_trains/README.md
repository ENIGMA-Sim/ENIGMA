# Use Case 7 — France High-Speed-Rail Mobility (step-by-step)

A second mobility scenario, built up **stage by stage** — the same four stages
the slide deck walks through.

5 TGV run along real **LGV** (*Ligne à Grande Vitesse*) corridors and report to
edge nodes in Paris, Lyon and Lille that feed the SNCF supervision fog server
and the cloud.

| Host | Corridor | Route (~km) |
|------|----------|-------------|
| `tgv_sud_est` | LGV Sud-Est / Méditerranée | Paris Gare de Lyon → Lyon Part-Dieu → Marseille St-Charles (~686) |
| `tgv_atlantique` | LGV Atlantique | Paris Montparnasse → Tours → Bordeaux St-Jean (~507) |
| `tgv_nord` | LGV Nord | Paris Gare du Nord → Lille Europe → Calais-Fréthun (~297) |
| `tgv_est` | LGV Est européenne | Paris Gare de l'Est → Reims → Strasbourg (~409) |
| `tgv_rhin_rhone` | LGV Rhin-Rhône | Dijon → Besançon → Belfort → Mulhouse (~185) |

Fixed hosts: `edge_paris`, `edge_lyon`, `edge_lille`, `fog_sncf`, `cloud_dc`.

---

## Stage 1 — the platform

[`france_rail_platform.xml`](france_rail_platform.xml) — SimGrid `4.1` platform,
`routing="Full"`. Declares the fixed hosts, one `<host>` per train, the links
(backbone / fog uplink / per-train GSM-R) and the routes. The mobility module
is switched on by a single zone property:

```xml
<zone id="france_rail" routing="Full">
  <prop id="mobility_dir" value="use_cases/07_mobility_france_trains/coords/"/>
  ...
  <host id="tgv_sud_est" speed="2Gf" core="2"/>
  ...
</zone>
```

## Stage 2 — the GPS traces

[`generate_traces.py`](generate_traces.py) holds one waypoint table of **real
SNCF station coordinates** per corridor, walks it at TGV cruise speed (~80–92
m/s) with ping-pong at the endpoints, and samples once per second:

```
coords/tgv_sud_est.csv
timestamp,latitude,longitude,speed,heading,pax_load,delay_s
0,48.844370,2.373440,92.0,0.0,60.0,30.0
1,48.843546,2.373701,92.0,197.5,60.1,30.1
...
```

The filename stem **must** equal the SimGrid host id. Only
`timestamp,latitude,longitude` are required; `speed`, `heading`, `pax_load`
(% seats) and `delay_s` are extra columns — interpolated and shown in the map
popups. Pure standard library, safe to re-run.

## Stage 3 — the simulation

`run.sh` runs `mobility_test_app` (built from
[`tests/mobility_test.cpp`](../../tests/mobility_test.cpp)):

```cpp
MobilityManager mob(e);                 // reads mobility_dir from the XML
mob.start_periodic_actor(e, 0.5);       // snapshot every train every 0.5 sim-s

for (auto* h : e.get_all_hosts())
    if (mob.has_trace(h->get_cname()))
        h->add_actor("mobile_actor", MobileActor(h->get_cname(), &mob));

e.run();
mob.export_json("snapshots.json");      // + export_csv / export_traces_json
```

Each `MobileActor` calls `mob.position_at("tgv_sud_est", now)` and reads the
interpolated `pos->latitude / longitude` and `pos->extra["pax_load"]`,
`pos->extra["delay_s"]`.

## Stage 4 — visualise

```bash
python3 src/python/tools/mobility_viewer.py /tmp/enigma_uc7_snapshots.json --offline
```

One polyline per corridor on OpenStreetMap, a time slider (step = recording
interval) to animate the trains, and clickable dots showing speed / heading /
passenger load / delay at that instant.

---

## Run everything

```bash
./use_cases/07_mobility_france_trains/run.sh
#   -> /tmp/enigma_uc7_snapshots.{json,csv} + _raw_traces.json  (~72 000 snapshots)
```

Override the output location with `OUT_PREFIX=/my/path ./run.sh`.

## Try next

- Add the LGV Interconnexion Est or a Thalys/Eurostar leg: append a waypoint
  table + `ROUTES` entry and a matching `<host>`.
- Add an actor that hands a train off from `edge_paris` to `edge_lyon` when it
  passes Mâcon, and logs the accumulated `delay_s` to the fog.
- Replace the synthetic traces with real GTFS-RT / SNCF open-data feeds
  (one CSV per train number).
