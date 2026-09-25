# Use Case 2 — Fog Analytics

## Scenario

Edge **data sources** generate 5–10 samples each and stream them to a
**fog analyzer**. The fog node:

1. runs a 100 MFlop real-time analysis per incoming sample,
2. after every source sends its `FIN` marker, runs a final aggregate pass
   (50 MFlop × number of samples),
3. reports count / mean / sum.

## Platform

`platform_generator fog-cluster 4 3` - 4 fog clusters × 3 nodes (12 hosts,
10 Gf / 1 GBps, 4 cores). The app uses ~1/3 of the hosts as analyzers and the
rest as data sources, round-robin.

## Run

```bash
./use_cases/02_fog_analytics/run.sh
```

## What to look for

```
[... fog_analyzer ...] === Analysis Results ===
[... fog_analyzer ...]   Samples processed: N
[... fog_analyzer ...]   Average: ...
=== Simulation completed ===
```

## Application (C++)

The template built and run by `run.sh` lives right here:
[`fog_analytics.cpp`](fog_analytics.cpp). `CMakeLists.txt` compiles it
straight from this directory into the `fog_analytics_app` binary.

## Energy

`platform_generator` attaches SimGrid's `host_energy` plugin properties
(`wattage_per_state` / `wattage_off`) to every host it writes. The app
activates the plugin with `sg_host_energy_plugin_init()` before
`e.load_platform()`, then after `e.run()` prints a per-host and total
consumption report (in Joules and kWh). Fog analyzers draw more than data
sources — they stay busy running the real-time analysis on every sample plus
the final aggregate pass.

## Try next

- Increase samples per source in [`fog_analytics.cpp`](fog_analytics.cpp)
  (`int samples = 5 + (i % 5);`).
- Compare `Simulated time` (and the energy report) with use case 1 — fog
  nodes are 10× faster but the extra hop adds latency.
