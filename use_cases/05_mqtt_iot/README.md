# Use Case 5 — MQTT Publish/Subscribe for IoT

## Scenario

Uses the **`enigma_mqtt`** communication module (`MQTTBroker`,
`MQTTPublisher`, `MQTTSubscriber`):

1. an **MQTT broker** actor starts on the first `edge` host;
2. **IoT sensors** publish 5 telemetry messages each to `sensors/temperature`;
3. **edge gateways** subscribe to `sensors/temperature`, filter/process the
   messages and (if any fog node exists) republish to `edge/filtered`.

## Platform

`platform_generator edge-cluster 2 4` → 8 hosts named `edge_cluster_*`. The app
auto-detects them: host 0 → broker, next 2 → sensors, rest → gateways. No fog
tier here, so gateways process locally.

## Run

```bash
./use_cases/05_mqtt_iot/run.sh
```

## What to look for

```
[SENSOR] Publishing: sensor=...,value=22.5,timestamp=...
[EDGE] Messages processed: 10/10
[EDGE] Processing completed (10 messages)
=== Simulation completed ===
```

> The broker actor runs an infinite receive loop, so SimGrid prints
> `1 actor is still active` for `mqtt_broker` at the end — that is expected,
> every sensor and gateway has finished.

## Application (C++)

The template built and run by `run.sh` lives right here:
[`mqtt_edge_app.cpp`](mqtt_edge_app.cpp). `CMakeLists.txt` compiles it
straight from this directory into the `mqtt_edge_app` binary.

## Energy

`platform_generator` attaches SimGrid's `host_energy` plugin properties
(`wattage_per_state` / `wattage_off`) to every host it writes. The app
activates the plugin with `sg_host_energy_plugin_init()` before
`e.load_platform()`, then after `e.run()` prints a per-host report tagging
the broker separately from the sensor/gateway devices, plus the total in
Joules and kWh.

## Try next

- Add a fog tier (`hybrid-cluster 1 4 1 2 0 0`) to see the
  `Sensors → Gateways → Fog` path (note the sample app's fog stage has a
  known timing quirk — inspect the source).
- Change the publish count in [`mqtt_edge_app.cpp`](mqtt_edge_app.cpp)
  (`IoTSensor(..., 5)`).
