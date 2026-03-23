"""
mqtt_edge_app.py – MQTT-based Edge-Fog simulation.

Mirrors tests/mqtt_edge_app.cpp.

Usage
-----
    python mqtt_edge_app.py <platform_file.xml>

Example
-------
    python mqtt_edge_app.py platforms/edge_platform.xml
"""
from __future__ import annotations

import sys
import os

# Allow running from the tests/ directory
_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.join(_HERE, "..")
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

import simgrid
from enigma.comms.mqtt import MQTTPublisher, MQTTSubscriber, start_broker


# --------------------------------------------------------------------------- #
# Actors                                                                       #
# --------------------------------------------------------------------------- #

class IoTSensor:
    """Publishes periodic temperature readings to the MQTT broker."""

    def __init__(self, topic: str, num_readings: int = 5) -> None:
        self._topic = topic
        self._num_readings = num_readings

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(
            f"[SENSOR] '{host.name}' started → publishing to '{self._topic}'"
        )

        publisher = MQTTPublisher("mqtt_broker")

        for i in range(self._num_readings):
            simgrid.this_actor.sleep_for(2.0)
            value   = 20.0 + i * 2.5
            payload = (
                f"sensor={host.name},"
                f"value={value:.2f},"
                f"timestamp={simgrid.Engine.clock:.3f}"
            )
            simgrid.this_actor.info(f"[SENSOR] Publishing: {payload}")
            publisher.publish(self._topic, payload, 100)
            simgrid.this_actor.execute(1e8)   # 100 MFlops local processing

        simgrid.this_actor.info(f"[SENSOR] Completed {self._num_readings} readings")


class EdgeGateway:
    """Receives sensor data, filters locally, optionally forwards to Fog."""

    def __init__(
        self,
        subscribe_topic: str,
        publish_topic: str,
        expected_messages: int,
        forward_to_fog: bool = True,
    ) -> None:
        self._sub_topic   = subscribe_topic
        self._pub_topic   = publish_topic
        self._expected    = expected_messages
        self._forward     = forward_to_fog

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(
            f"[EDGE] '{host.name}' started → subscribing to '{self._sub_topic}'"
        )

        subscriber  = MQTTSubscriber("mqtt_broker")
        publisher   = MQTTPublisher("mqtt_broker")

        subscriber.subscribe(self._sub_topic)

        received = 0
        while received < self._expected:
            msg = subscriber.receive(5.0)
            if msg is None:
                simgrid.this_actor.info(
                    f"[EDGE] Timeout – processed {received}/{self._expected} messages"
                )
                break

            simgrid.this_actor.info(
                f"[EDGE] Received from '{msg.topic}': {msg.payload}"
            )
            simgrid.this_actor.info("[EDGE] Filtering data locally…")
            simgrid.this_actor.execute(2e8)   # 200 MFlops

            if self._forward:
                filtered = f"filtered:{msg.payload}"
                publisher.publish(self._pub_topic, filtered, 150)
                simgrid.this_actor.info(
                    f"[EDGE] Forwarded to fog (topic: {self._pub_topic})"
                )
            else:
                simgrid.this_actor.info("[EDGE] Processed locally (no fog nodes)")

            received += 1
            simgrid.this_actor.info(f"[EDGE] Messages processed: {received}/{self._expected}")

        simgrid.this_actor.info(f"[EDGE] Processing completed ({received} messages)")


class FogAggregator:
    """Subscribes to edge-filtered data and performs aggregation."""

    def __init__(self, subscribe_topic: str, expected_messages: int, timeout: float = 10.0) -> None:
        self._topic    = subscribe_topic
        self._expected = expected_messages
        self._timeout  = timeout

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(
            f"[FOG] '{host.name}' started → subscribing to '{self._topic}'"
        )

        subscriber = MQTTSubscriber("mqtt_broker")
        subscriber.subscribe(self._topic)

        received: list[str] = []
        while len(received) < self._expected:
            msg = subscriber.receive(self._timeout)
            if msg is None:
                simgrid.this_actor.info("[FOG] Timeout – stopping aggregation")
                break

            simgrid.this_actor.info(f"[FOG] Received: {msg.payload}")
            received.append(msg.payload)
            simgrid.this_actor.execute(5e8)   # 500 MFlops

        simgrid.this_actor.info(
            f"[FOG] Final aggregation of {len(received)} data sets…"
        )
        simgrid.this_actor.execute(2e9)   # 2 GFlops
        simgrid.this_actor.info("[FOG] Aggregation completed")


# --------------------------------------------------------------------------- #
# main                                                                         #
# --------------------------------------------------------------------------- #

def main() -> None:
    e = simgrid.Engine(sys.argv)

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <platform_file.xml>", file=sys.stderr)
        sys.exit(1)

    e.load_platform(sys.argv[1])
    hosts = e.all_hosts

    if len(hosts) < 3:
        print("Error: at least 3 hosts are required.", file=sys.stderr)
        sys.exit(1)

    simgrid.this_actor.info("=== MQTT Edge Computing Application ===")
    simgrid.this_actor.info(f"Platform loaded with {len(hosts)} hosts")

    # Choose broker host: prefer fog (reachable from all tiers), then edge, then first
    broker_host = (
        next((h for h in hosts if "fog" in h.name), None)
        or next((h for h in hosts if "cloud" in h.name), None)
        or next((h for h in hosts if "edge" in h.name), hosts[0])
    )
    simgrid.this_actor.info(f"Starting MQTT Broker on host '{broker_host.name}'")
    start_broker(broker_host, "mqtt_broker")

    # Classify remaining hosts
    sensors:    list[simgrid.Host] = []
    gateways:   list[simgrid.Host] = []
    fog_nodes:  list[simgrid.Host] = []

    for host in hosts:
        if host is broker_host:
            continue
        name = host.name
        if "edge" in name:
            if len(sensors) < 2:
                sensors.append(host)
            else:
                gateways.append(host)
        elif "fog" in name:
            fog_nodes.append(host)

    # Fallback when host names do not contain tier keywords
    if not sensors and not gateways:
        sensors = list(hosts[1:3])
        if len(hosts) > 3:
            gateways = [hosts[3]]
        else:
            gateways = [sensors[-1]]
            sensors  = sensors[:-1]
        if len(hosts) > 4:
            fog_nodes = [hosts[4]]

    if not gateways and sensors:
        gateways = [sensors.pop()]

    has_fog = bool(fog_nodes)
    simgrid.this_actor.info(
        f"Sensors: {len(sensors)}, Gateways: {len(gateways)}, Fog: {len(fog_nodes)}"
    )

    # Deploy sensors
    for host in sensors:
        simgrid.Actor.create("iot_sensor", host,
                             IoTSensor("sensors/temperature", 5))

    # Deploy edge gateways
    for host in gateways:
        simgrid.Actor.create(
            "edge_gateway", host,
            EdgeGateway(
                "sensors/temperature", "edge/filtered",
                len(sensors) * 5,
                has_fog,
            ),
        )

    # Deploy fog aggregators
    for host in fog_nodes:
        simgrid.Actor.create(
            "fog_aggregator", host,
            FogAggregator("edge/filtered", len(gateways) * len(sensors) * 5),
        )

    e.run()
    simgrid.this_actor.info("=== Simulation completed ===")
    simgrid.this_actor.info(f"Simulated time: {simgrid.Engine.clock:.2f} s")


if __name__ == "__main__":
    main()
