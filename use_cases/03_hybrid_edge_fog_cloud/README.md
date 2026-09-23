# Use Case 3 — Hybrid Edge-Fog-Cloud Pipeline

## Scenario

A classic three-tier IoT pipeline:

| Tier | Actor | Work |
|------|-------|------|
| Edge | `EdgeCollector` | collect + locally filter data (200 MFlop ×3), push 500 KB to fog |
| Fog | `FogAggregator` | pre-process each message (500 MFlop), aggregate (2 GFlop), push 1 MB summary to cloud |
| Cloud | `CloudProcessor` | store each summary (1 GFlop), final analytics / ML (10 GFlop) |

## Platform

`platform_generator hybrid-cluster 2 4 1 2 1 1 1 hybrid_platform.xml`

- 2 edge clusters × 4 nodes (1 Gf)
- 1 fog cluster × 2 nodes (10 Gf)
- 1 cloud cluster × 1 node (100 Gf)
- trailing `1` = add direct Edge↔Cloud links (2 GBps / 30 ms) on top of the
  Edge→Fog→Cloud topology

The app classifies hosts by the substrings `edge` / `fog` / `cloud` in their
names.

## Run

```bash
./use_cases/03_hybrid_edge_fog_cloud/run.sh
```

## What to look for

```
[... CLOUD ...] Received summary: fog_summary_12
[... CLOUD ...] Performing complete analysis and machine learning...
=== Edge-Fog-Cloud pipeline completed successfully ===
```

## Application (C++)

The template built and run by `run.sh` lives right here:
[`hybrid_cloud.cpp`](hybrid_cloud.cpp). `CMakeLists.txt` compiles it straight
from this directory into the `hybrid_cloud_app` binary.

## Energy

`platform_generator` attaches SimGrid's `host_energy` plugin properties
(`wattage_per_state` / `wattage_off`) to every host it writes. The app
activates the plugin with `sg_host_energy_plugin_init()` before
`e.load_platform()`, then after `e.run()` prints a report broken down **by
tier**:

```
=== Energy Report ===
  [EDGE ] edge_cluster_0_node_0       593.28 J
  ...
  [FOG  ] fog_cluster_0_node_0        655.88 J
  ...
  [CLOUD] cloud_cluster_0_node_0      869.57 J
  ------------------------------------------------------
  Edge tier total :    4746.24 J
  Fog tier total  :    1311.77 J
  Cloud tier total:     869.57 J
  Total energy consumed: 6927.58 J (0.001924 kWh)
```

Useful to compare against use case 4 (offloading): pushing work up the tiers
trades edge energy for fog/cloud energy plus network cost.

## Try next

- Drop the direct Edge-Cloud links: change the trailing `1` to `0`.
- Scale the cloud tier: `... 1 3 ...` (note: the sample app expects a single
  cloud consumer — see the source before scaling).
