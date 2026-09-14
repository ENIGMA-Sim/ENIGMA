# Use Case 2 — Fog Analytics

## Scenario

Edge **data sources** generate 5–10 samples each and stream them to a
**fog analyzer**. The fog node:

1. runs a 100 MFlop real-time analysis per incoming sample,
2. after every source sends its `FIN` marker, runs a final aggregate pass
   (50 MFlop × number of samples),
3. reports count / mean / sum.

## Platform

`platform_generator fog-cluster 4 3` → 4 fog clusters × 3 nodes (12 hosts,
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

## Try next

- Increase samples per source in
  [`tests/fog_analytics.cpp`](../../tests/fog_analytics.cpp)
  (`int samples = 5 + (i % 5);`).
- Compare `Simulated time` with use case 1 — fog nodes are 10× faster but the
  extra hop adds latency.
