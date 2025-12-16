# MQTT Implementation Summary

### Core Components

1. **MQTTBroker** (`include/mqtt/MQTTBroker.hpp`, `src/mqtt/MQTTBroker.cpp`)
   - Central message broker managing topics and subscriptions
   - SimGrid actor running on designated host
   - Handles subscribe, unsubscribe, publish, and shutdown messages
   - Tracks statistics (messages published/delivered)
   - Topic-based message routing to subscribers

2. **MQTTPublisher** (`include/mqtt/MQTTPublisher.hpp`, `src/mqtt/MQTTPublisher.cpp`)
   - Client for publishing messages to topics
   - Auto-generates publisher ID from host and actor ID
   - Supports explicit or automatic message size
   - QoS support (0, 1, 2)
   - Sends messages to broker via control mailbox

3. **MQTTSubscriber** (`include/mqtt/MQTTSubscriber.hpp`, `src/mqtt/MQTTSubscriber.cpp`)
   - Client for subscribing to topics
   - Auto-generates subscriber ID
   - Subscribe/unsubscribe operations
   - Blocking receive with optional timeout
   - Message availability checking

4. **MQTT Convenience Header** (`include/mqtt/MQTT.hpp`)
   - Single include for all MQTT functionality
   - Helper functions: `start_broker()`, `create_publisher()`, `create_subscriber()`
   - `MQTTConfig` struct for configuration
   - Easy enable/disable MQTT in applications

### Message Structure

```cpp
struct MQTTMessage {
    std::string topic;       // "sensors/temperature", "edge/filtered", etc.
    std::string payload;     // Message content
    size_t size;            // Bytes
    double timestamp;       // SimGrid clock
    std::string publisher;  // Publisher ID
    int qos;               // Quality of Service (0, 1, 2)
};
```

### Build Integration

- Added `enigma_mqtt` library to CMakeLists.txt
- All applications linked with MQTT library
- New executable: `mqtt_edge_app`

### Example Application

**mqtt_edge_app** (`src/apps/mqtt_edge_app.cpp`):
- Complete IoT → Edge → Fog pipeline using MQTT
- IoT Sensors publish to `sensors/temperature`
- Edge Gateways subscribe, filter, and republish to `edge/filtered`
- Fog Aggregators subscribe to `edge/filtered` and aggregate
- Demonstrates full publish/subscribe pattern


## How to Use

### 1. Basic Usage

```cpp
#include "mqtt/MQTT.hpp"
using namespace enigma::mqtt;

// Start broker
start_broker(host, "mqtt_broker");

// Publish
MQTTPublisher pub("mqtt_broker");
pub.publish("sensors/temp", "25.5°C");

// Subscribe
MQTTSubscriber sub("mqtt_broker");
sub.subscribe("sensors/temp");
auto msg = sub.receive();
```

### 2. Integration Patterns

**Pattern A: Pure MQTT** - All communication through MQTT
```cpp
MQTTPublisher pub;
pub.publish(topic, data);
```

**Pattern B: Hybrid** - Both MQTT and direct mailbox
```cpp
if (use_mqtt) {
    pub.publish(topic, data);
} else {
    mbox->put(data, size);
}
```

**Pattern C: Optional** - MQTT disabled by default
```cpp
MQTTConfig config = MQTTConfig::create_disabled();
if (enable_mqtt) {
    config = MQTTConfig::create_enabled("broker_host");
}
```

### 3. Running Example

```bash
# Build
cd build && cmake .. && make -j$(nproc)

# Generate platform
./build/platform_generator edge-cluster 1 5

# Run MQTT application
./build/mqtt_edge_app ./platforms/edge_platform.xml
```

Output snippet:
```
[INFO] MQTT Broker 'mqtt_broker' started on host 'edge_cluster_0_node_0'
[INFO] Subscriber 'edge_cluster_0_node_3_sub_4' subscribed to topic 'sensors/temperature' (1 total subscribers)
[INFO] [SENSOR] Publishing: sensor=edge_cluster_0_node_1,value=20.000000,timestamp=2.000000
[INFO] Publishing message to topic 'sensors/temperature' (size: 63 bytes)
[INFO] [EDGE] Received from topic 'sensors/temperature': sensor=...
[INFO] Message delivered to 2 subscribers
```

## Architecture

```
IoT Sensors          MQTT Broker         Edge Gateways         Fog Nodes
(Publishers)      (Central Router)       (Sub+Pub)          (Subscribers)
     │                   │                   │                   │
     │  publish()        │                   │                   │
     ├──────────────────>│                   │                   │
     │                   │  deliver()        │                   │
     │                   ├──────────────────>│                   │
     │                   │                   │  publish()        │
     │                   │<──────────────────┤                   │
     │                   │                   │  deliver()        │
     │                   ├──────────────────────────────────────>│
     │                   │                   │                   │
```
