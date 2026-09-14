#!/usr/bin/env bash
# Use case 5 – MQTT publish/subscribe for IoT
# Sensor actors publish telemetry to an MQTT broker; gateway/analytics actors
# subscribe to the topics and consume the messages. Uses the enigma_mqtt module
# (MQTTBroker / MQTTPublisher / MQTTSubscriber).
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

ensure_built platform_generator mqtt_edge_app

# 2 edge clusters x 4 nodes. The app auto-detects "edge" hosts and starts the
# MQTT broker on the first one, then splits the rest into sensors / gateways.
gen_platform edge-cluster 2 4

run_enigma mqtt_edge_app platforms/edge_platform.xml
