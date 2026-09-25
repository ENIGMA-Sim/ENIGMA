# Use Case 1 — Edge Computing

## Scenario

A set of **edge devices** each:

1. collect sensor data (1 s),
2. process it locally (`sg4::this_actor::execute`, 1 GFlop),
3. send the result to an **edge gateway**.

The gateway receives every result and runs a 500 MFlop aggregation step per
message. No fog or cloud tier is involved — this is the "process at the edge"
baseline.

## Platform

`platform_generator edge-cluster 3 5` → 3 edge clusters × 5 nodes (15 hosts,
1 Gf / 125 MBps each). The application picks one host as the gateway and treats
the rest as devices.

## Run

```bash
./use_cases/01_edge_computing/run.sh
```

## What to look for

```
[... gateway ...] Received: Processed data from edge_cluster_1_node_0
[... gateway ...] All data processed. Gateway completed.
=== Simulation completed ===
Simulated time: ~9.8 seconds
```

`Simulated time` is SimGrid's model time, not wall-clock — it reflects the
compute + network latencies declared in the platform.

## Application (C++)

The template built and run by `run.sh` lives right here:
[`edge_computing.cpp`](edge_computing.cpp). `CMakeLists.txt` compiles it
straight from this directory into the `edge_computing_app` binary — edit it,
re-run `run.sh`, and it recompiles automatically.

## Energy

`platform_generator` attaches SimGrid's `host_energy` plugin properties
(`wattage_per_state` / `wattage_off`) to every host it writes. The app
activates the plugin with `sg_host_energy_plugin_init()` **before**
`e.load_platform()`, then after `e.run()` prints a per-host and total
consumption report:

```
=== Energy Report ===
  edge_cluster_0_node_0             1015.22 J
  edge_cluster_0_node_1              925.22 J
  ...
  ------------------------------------------------------
  Total energy consumed: 13968.28 J (0.003880 kWh)
```

The gateway draws more because it stays busier (it aggregates every message);
idle devices still consume their idle wattage for the full simulated time.

## Try next

- `edge-cluster 5 10` for a bigger fleet.
- Edit [`edge_computing.cpp`](edge_computing.cpp) — `comp_size` / `data_size`
  in `main()` — then re-run (the script recompiles).
