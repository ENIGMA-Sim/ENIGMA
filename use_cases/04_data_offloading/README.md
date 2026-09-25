# Use Case 4 — Smart Data Offloading

## Scenario

Each **edge device** generates several tasks. For every task it compares
its own CPU capacity with the task workload and decides where to run it:

| Condition | Decision |
|-----------|----------|
| `workload < 0.5 × capacity` | **process locally** |
| `0.5 × capacity ≤ workload < 2 × capacity` | **offload to Fog**, wait for the result |
| `workload ≥ 2 × capacity` | **offload to Cloud**, wait for the result |

Fog and Cloud servers are request/response: they execute the workload and send
back the measured latency. Servers self-terminate on a 10 s idle timeout.

Devices are created with an increasing workload (`1e9 × (0.5 + i·0.5)`), so the
run exercises all three branches.

## Platform

`platform_generator hybrid-cluster 2 4 2 2 1 1 1 hybrid_platform.xml` — 8 edge,
4 fog, 1 cloud host. `run.sh` passes `5` as the 2nd argument = tasks per device.

## Run

```bash
./use_cases/04_data_offloading/run.sh
```

## What to look for

```
Decision: LOCAL PROCESSING (low load)
Decision: OFFLOAD TO FOG (medium load)
Decision: OFFLOAD TO CLOUD (high load)
[FOG]  Task processed in ... seconds
[CLOUD] No more tasks, finishing (processed N tasks)
=== Simulation completed ===
```

## Application (C++)

The template built and run by `run.sh` lives right here:
[`data_offloading.cpp`](data_offloading.cpp). `CMakeLists.txt` compiles it
straight from this directory into the `data_offloading_app` binary.

## Energy

`platform_generator` attaches SimGrid's `host_energy` plugin properties
(`wattage_per_state` / `wattage_off`) to every host it writes. The app
activates the plugin with `sg_host_energy_plugin_init()` before
`e.load_platform()`, then after `e.run()` prints a per-tier energy report
(Edge / Fog / Cloud) plus the grand total in Joules and kWh - a direct way to
see the energy cost of each offloading decision: devices that keep work
local spend less on the network but more on their own (slower) CPU, while
offloaded tasks shift the energy bill to the fog/cloud tier.

## Try next

- Change the tasks-per-device count: edit the last argument in `run.sh`.
- Make every device offload: raise `base_workload` in
  [`data_offloading.cpp`](data_offloading.cpp).
