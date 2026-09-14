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

## Try next

- Add a fog tier (`hybrid-cluster 1 4 1 2 0 0`) to see the
  `Sensors → Gateways → Fog` path (note the sample app's fog stage has a
  known timing quirk — inspect the source).
- Change the publish count in
  [`tests/mqtt_edge_app.cpp`](../../tests/mqtt_edge_app.cpp) (`IoTSensor(..., 5)`).
