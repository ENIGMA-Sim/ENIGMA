# MQTT Communication Protocol for ENIGMA

## Overview

ENIGMA now includes a complete MQTT (Message Queuing Telemetry Transport) implementation for publish/subscribe communication patterns. This allows IoT sensors, edge devices, and cloud nodes to communicate asynchronously through topics.

## Features

- **🔌 Publish/Subscribe Pattern**: Decoupled communication between producers and consumers
- **📡 Topic-Based Routing**: Messages are routed based on topic names (e.g., "sensors/temperature")
- **🚀 Async Communication**: Non-blocking message delivery through SimGrid mailboxes
- **📊 Broker Statistics**: Track messages published and delivered
- **⚙️ Quality of Service (QoS)**: Support for QoS levels 0, 1, 2
- **🔧 Easy Integration**: Simple API to add MQTT to existing applications

## Architecture

```
┌─────────────┐                ┌─────────────┐                ┌─────────────┐
│             │  publish()     │             │  deliver()     │             │
│  Publisher  │───────────────▶│    Broker   │───────────────▶│ Subscriber  │
│             │                │             │                │             │
└─────────────┘                └─────────────┘                └─────────────┘
                                      │
                                      │ manage topics &
                                      │ subscriptions
                                      ▼
                               ┌──────────────┐
                               │   Topics     │
                               │ - sensors/*  │
                               │ - edge/data  │
                               │ - fog/agg    │
                               └──────────────┘
```

### Components

1. **MQTTBroker**: Central message broker that manages topics and subscriptions
2. **MQTTPublisher**: Client for publishing messages to topics
3. **MQTTSubscriber**: Client for subscribing to topics and receiving messages
4. **MQTTMessage**: Message structure with metadata (timestamp, publisher, QoS)

## Quick Start

### 1. Include MQTT Headers

```cpp
#include "mqtt/MQTT.hpp"

using namespace enigma::mqtt;
```

### 2. Start MQTT Broker

The broker must be started on one host in your platform:

```cpp
// Start broker on first host
sg4::Host* broker_host = hosts[0];
start_broker(broker_host, "mqtt_broker");
```

### 3. Create Publishers

IoT devices or edge nodes publish data:

```cpp
class IoTSensor {
public:
    void operator()() {
        // Create publisher
        MQTTPublisher publisher("mqtt_broker");
        
        // Publish sensor data
        std::string data = "temperature=25.5,humidity=60";
        publisher.publish("sensors/temperature", data, 100);  // 100 bytes
    }
};
```

### 4. Create Subscribers

Edge gateways or fog nodes subscribe to receive data:

```cpp
class EdgeGateway {
public:
    void operator()() {
        // Create subscriber
        MQTTSubscriber subscriber("mqtt_broker");
        
        // Subscribe to topic
        subscriber.subscribe("sensors/temperature");
        
        // Receive messages
        while (true) {
            auto msg = subscriber.receive();
            if (msg) {
                XBT_INFO("Received: %s", msg->payload.c_str());
            }
        }
    }
};
```

## API Reference

### MQTTBroker

The central message broker.

```cpp
// Constructor
MQTTBroker(const std::string& name = "mqtt_broker");

// Start broker (helper function)
simgrid::s4u::ActorPtr start_broker(simgrid::s4u::Host* host,
                                    const std::string& broker_name = "mqtt_broker");
```

### MQTTPublisher

Publishes messages to topics.

```cpp
// Constructor
MQTTPublisher(const std::string& broker = "mqtt_broker",
              const std::string& publisher_id = "");

// Publish with explicit size
void publish(const std::string& topic,
             const std::string& payload,
             size_t size,
             int qos = 0);

// Publish with auto size
void publish(const std::string& topic,
             const std::string& payload,
             int qos = 0);
```

### MQTTSubscriber

Subscribes to topics and receives messages.

```cpp
// Constructor
MQTTSubscriber(const std::string& broker = "mqtt_broker",
               const std::string& subscriber_id = "");

// Subscribe to topic
void subscribe(const std::string& topic);

// Unsubscribe from topic
void unsubscribe(const std::string& topic);

// Receive message (blocking)
std::shared_ptr<MQTTMessage> receive(double timeout = -1);

// Check if messages available
bool has_messages() const;
```

### MQTTMessage

Message structure with metadata.

```cpp
struct MQTTMessage {
    std::string topic;       // Topic name
    std::string payload;     // Message content
    size_t size;            // Size in bytes
    double timestamp;       // SimGrid clock time
    std::string publisher;  // Publisher ID
    int qos;               // Quality of Service
};
```

## Complete Example

See `src/apps/mqtt_edge_app.cpp` for a complete example:

```cpp
#include "mqtt/MQTT.hpp"

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    e.load_platform(argv[1]);
    
    auto hosts = e.get_all_hosts();
    
    // Start MQTT Broker
    start_broker(hosts[0], "mqtt_broker");
    
    // Create IoT Sensors (publishers)
    simgrid::s4u::Actor::create("sensor", hosts[1], IoTSensor("sensors/temp", 5));
    
    // Create Edge Gateway (subscriber)
    simgrid::s4u::Actor::create("gateway", hosts[2], EdgeGateway("sensors/temp"));
    
    e.run();
    return 0;
}
```

## Topic Naming Conventions

Recommended topic hierarchy:

```
sensors/            # IoT sensor data
├── temperature
├── humidity
└── pressure

edge/               # Edge processing results
├── filtered
├── aggregated
└── alerts

fog/                # Fog processing
├── analytics
└── predictions

cloud/              # Cloud services
├── storage
└── ml_results
```

## Quality of Service (QoS) Levels

- **QoS 0** (At most once): Fire and forget, no acknowledgment
- **QoS 1** (At least once): Message delivered at least once
- **QoS 2** (Exactly once): Message delivered exactly once

*Note: Current implementation provides basic QoS support for future extensions.*

## Integration with Existing Applications

### Option 1: Add MQTT to Existing App

Modify your existing applications to use MQTT:

```cpp
// Before (direct mailbox)
auto* mbox = sg4::Mailbox::by_name("destination");
mbox->put(data, size);

// After (MQTT publish)
MQTTPublisher pub("mqtt_broker");
pub.publish("edge/data", data, size);
```

### Option 2: Keep Both Modes

Use configuration to enable/disable MQTT:

```cpp
bool use_mqtt = true;  // Set via command line or config

if (use_mqtt) {
    MQTTPublisher pub;
    pub.publish(topic, data);
} else {
    auto* mbox = sg4::Mailbox::by_name(dest);
    mbox->put(data, size);
}
```

### Option 3: Wrapper Function

Create a wrapper for transparent switching:

```cpp
void send_data(const std::string& dest, const std::string& data, bool use_mqtt) {
    if (use_mqtt) {
        MQTTPublisher pub;
        pub.publish(dest, data);
    } else {
        auto* mbox = sg4::Mailbox::by_name(dest);
        mbox->put(new std::string(data), data.size());
    }
}
```

## Performance Considerations

- **Broker Placement**: Place broker on a well-connected host
- **Message Size**: Larger messages increase transfer time
- **Topic Granularity**: Balance between specific and general topics
- **Subscriber Count**: More subscribers = more message copies

## Testing

Run the MQTT example application:

```bash
# Generate platform
./build/platform_generator edge-cluster 1 5

# Run MQTT application
./build/mqtt_edge_app ./platforms/edge_platform.xml
```

Expected output:
```
[INFO] MQTT Broker 'mqtt_broker' started
[INFO] [SENSOR] Publishing to topic 'sensors/temperature'
[INFO] [EDGE] Received from topic 'sensors/temperature': sensor=...
[INFO] Message delivered to 2 subscribers
```

## Advanced Topics

### Multiple Brokers

You can run multiple brokers with different names:

```cpp
start_broker(host1, "broker_edge");
start_broker(host2, "broker_cloud");

// Connect to specific broker
MQTTPublisher pub1("broker_edge");
MQTTPublisher pub2("broker_cloud");
```

### Topic Wildcards (Future)

Planned support for wildcard subscriptions:

```cpp
subscriber.subscribe("sensors/#");      // All sensor topics
subscriber.subscribe("edge/+/data");    // Single-level wildcard
```

### Retained Messages (Future)

Planned support for message retention:

```cpp
publisher.publish("sensors/config", "...", 0, true);  // retained=true
```

## Troubleshooting

### Broker Not Started

**Problem**: "Mailbox 'mqtt_broker_...' not found"

**Solution**: Ensure broker is started before publishers/subscribers:
```cpp
start_broker(host, "mqtt_broker");
sg4::this_actor::sleep_for(0.1);  // Small delay
```

### No Messages Received

**Problem**: Subscriber receives no messages

**Solution**:
1. Check topic names match exactly
2. Ensure subscriber subscribes before publishing
3. Verify broker is running

### Deadlock Warning

**Problem**: "Deadlock detected, some activities still waiting"

**Solution**: This is normal if actors wait indefinitely for messages. Use timeouts:
```cpp
auto msg = subscriber.receive(10.0);  // 10 second timeout
```

## References

- **MQTT Protocol**: https://mqtt.org/
- **SimGrid Documentation**: https://simgrid.org/
- **Example Application**: `src/apps/mqtt_edge_app.cpp`
- **Headers**: `include/mqtt/MQTT.hpp`

---

**Version**: 1.0.0  
**Status**: Production Ready ✅  
**Last Updated**: 2025-11-20
