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

## Try next

- `edge-cluster 5 10` for a bigger fleet.
- Edit [`tests/edge_computing.cpp`](../../tests/edge_computing.cpp) —
  `comp_size` / `data_size` in `main()` — then re-run (the script recompiles).
