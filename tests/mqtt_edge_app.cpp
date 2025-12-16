#include <simgrid/s4u.hpp>
#include "comms/mqtt/MQTT.hpp"
#include <iostream>
#include <string>

XBT_LOG_NEW_DEFAULT_CATEGORY(mqtt_example, "MQTT Example Application");

namespace sg4 = simgrid::s4u;
using namespace enigma::mqtt;

/**
 * @brief IoT Sensor using MQTT to publish data
 */
class IoTSensor {
    std::string topic;
    int num_readings;
    
public:
    IoTSensor(const std::string& sensor_topic, int readings = 5)
        : topic(sensor_topic), num_readings(readings) {}
    
    void operator()() {
        sg4::Host* host = sg4::this_actor::get_host();
        XBT_INFO("[SENSOR] '%s' started - publishing to topic '%s'",
                 host->get_cname(), topic.c_str());
        
        // Create MQTT publisher
        MQTTPublisher publisher("mqtt_broker");
        
        // Simulate sensor readings
        for (int i = 0; i < num_readings; i++) {
            sg4::this_actor::sleep_for(2.0);  // Sensor sampling interval
            
            // Generate sensor data
            double value = 20.0 + (i * 2.5);  // Temperature readings
            std::string payload = "sensor=" + std::string(host->get_cname()) + 
                                 ",value=" + std::to_string(value) +
                                 ",timestamp=" + std::to_string(sg4::Engine::get_clock());
            
            XBT_INFO("[SENSOR] Publishing: %s", payload.c_str());
            
            // Publish via MQTT
            publisher.publish(topic, payload, 100);  // 100 bytes
            
            // Local processing
            sg4::this_actor::execute(1e8);  // 100 MFlops
        }
        
        XBT_INFO("[SENSOR] Completed %d readings", num_readings);
    }
};

/**
 * @brief Edge Gateway using MQTT to receive and filter data
 */
class EdgeGateway {
    std::string subscribe_topic;
    std::string publish_topic;
    int expected_messages;
    bool forward_to_fog;  // Only forward if fog nodes exist
    
public:
    EdgeGateway(const std::string& sub_topic, const std::string& pub_topic, 
                int expected, bool forward = true)
        : subscribe_topic(sub_topic), publish_topic(pub_topic), 
          expected_messages(expected), forward_to_fog(forward) {}
    
    void operator()() {
        sg4::Host* host = sg4::this_actor::get_host();
        XBT_INFO("[EDGE] '%s' started - subscribing to '%s'",
                 host->get_cname(), subscribe_topic.c_str());
        
        // Create MQTT subscriber and publisher
        MQTTSubscriber subscriber("mqtt_broker");
        MQTTPublisher publisher("mqtt_broker");
        
        subscriber.subscribe(subscribe_topic);
        
        int messages_received = 0;
        
        while (messages_received < expected_messages) {
            // Receive sensor data
            auto msg = subscriber.receive(5.0);  // 5 second timeout
            
            if (msg) {
                XBT_INFO("[EDGE] Received from topic '%s': %s",
                         msg->topic.c_str(), msg->payload.c_str());
                
                // Filter data locally
                XBT_INFO("[EDGE] Filtering data...");
                sg4::this_actor::execute(2e8);  // 200 MFlops
                
                if (forward_to_fog) {
                    // Forward filtered data to fog
                    XBT_INFO("[EDGE] Forwarding to fog (topic: %s)", publish_topic.c_str());
                    std::string filtered = "filtered:" + msg->payload;
                    publisher.publish(publish_topic, filtered, 150);
                    
                    XBT_INFO("[EDGE] Forwarded filtered data to fog");
                } else {
                    XBT_INFO("[EDGE] Processed locally (no fog nodes)");
                }
                
                messages_received++;
                XBT_INFO("[EDGE] Messages processed: %d/%d", messages_received, expected_messages);
            } else {
                XBT_INFO("[EDGE] Timeout waiting for messages (received %d/%d)",
                         messages_received, expected_messages);
                break;
            }
        }
        
        XBT_INFO("[EDGE] Processing completed (%d messages)", messages_received);
    }
};

/**
 * @brief Fog Aggregator using MQTT
 */
class FogAggregator {
    std::string subscribe_topic;
    int expected_messages;
    double timeout;
    
public:
    FogAggregator(const std::string& sub_topic, int expected, double timeout_sec = 10.0)
        : subscribe_topic(sub_topic), expected_messages(expected), timeout(timeout_sec) {}
    
    void operator()() {
        sg4::Host* host = sg4::this_actor::get_host();
        XBT_INFO("[FOG] '%s' started - subscribing to '%s'",
                 host->get_cname(), subscribe_topic.c_str());
        
        MQTTSubscriber subscriber("mqtt_broker");
        subscriber.subscribe(subscribe_topic);
        
        int messages_received = 0;
        std::vector<std::string> aggregated_data;
        
        while (messages_received < expected_messages) {
            auto msg = subscriber.receive(timeout);
            
            if (msg) {
                XBT_INFO("[FOG] Received: %s", msg->payload.c_str());
                aggregated_data.push_back(msg->payload);
                
                // Aggregate and analyze
                sg4::this_actor::execute(5e8);  // 500 MFlops
                
                messages_received++;
            } else {
                XBT_INFO("[FOG] Timeout - stopping aggregation");
                break;
            }
        }
        
        // Final aggregation
        XBT_INFO("[FOG] Performing final aggregation of %zu data sets",
                 aggregated_data.size());
        sg4::this_actor::execute(2e9);  // 2 GFlops
        
        XBT_INFO("[FOG] Aggregation completed");
    }
};

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform_file.xml>", argv[0]);
        XBT_CRITICAL("Example: %s platforms/edge_platform.xml", argv[0]);
        return 1;
    }
    
    e.load_platform(argv[1]);
    
    std::vector<sg4::Host*> hosts = e.get_all_hosts();
    
    if (hosts.size() < 3) {
        XBT_CRITICAL("At least 3 hosts required (Broker, Sensor, Gateway)");
        return 1;
    }
    
    XBT_INFO("=== MQTT Edge Computing Application ===");
    XBT_INFO("Platform loaded with %zu hosts", hosts.size());
    
    // Start MQTT Broker on an edge host for best compatibility
    // Edge hosts are typically well-connected and avoid routing issues
    sg4::Host* broker_host = nullptr;
    for (auto* host : hosts) {
        if (std::string(host->get_cname()).find("edge") != std::string::npos) {
            broker_host = host;
            break;
        }
    }
    // Fallback to first host if no edge hosts found
    if (broker_host == nullptr) {
        broker_host = hosts[0];
    }
    
    XBT_INFO("Starting MQTT Broker on host '%s'", broker_host->get_cname());
    start_broker(broker_host, "mqtt_broker");
    
    // Classify hosts
    std::vector<sg4::Host*> sensors, gateways, fog_nodes;
    
    for (auto* host : hosts) {
        std::string name = host->get_cname();
        // Skip broker host
        if (host == broker_host) continue;
        
        if (name.find("edge") != std::string::npos && host != broker_host) {
            if (sensors.size() < 2) {
                sensors.push_back(host);
            } else {
                gateways.push_back(host);
            }
        } else if (name.find("fog") != std::string::npos) {
            fog_nodes.push_back(host);
        }
    }
    
    // Fallback: use all hosts if no specific names
    if (sensors.empty() && gateways.empty()) {
        for (size_t i = 1; i < hosts.size() && i <= 2; i++) {
            sensors.push_back(hosts[i]);
        }
        if (hosts.size() > 3) {
            gateways.push_back(hosts[3]);
        } else {
            gateways.push_back(hosts[1]);
        }
        if (hosts.size() > 4) {
            fog_nodes.push_back(hosts[4]);
        }
    }
    
    // Ensure we have at least minimum hosts
    if (gateways.empty() && !sensors.empty()) {
        gateways.push_back(sensors.back());
        sensors.pop_back();
    }
    
    XBT_INFO("Configuration:");
    XBT_INFO("  Sensors: %zu", sensors.size());
    XBT_INFO("  Gateways: %zu", gateways.size());
    XBT_INFO("  Fog nodes: %zu", fog_nodes.size());
    
    // Determine if we have fog nodes
    bool has_fog = !fog_nodes.empty();
    
    // Start IoT Sensors (publish to "sensors/temperature")
    for (auto* host : sensors) {
        host->add_actor("iot_sensor", IoTSensor("sensors/temperature", 5));
    }
    
    // Start Edge Gateways (subscribe to "sensors/temperature", publish to "edge/filtered")
    for (auto* host : gateways) {
        host->add_actor("edge_gateway",
                        EdgeGateway("sensors/temperature", "edge/filtered",
                                    sensors.size() * 5, has_fog));  // forward_to_fog = has_fog
    }
    
    // Start Fog Aggregators (subscribe to "edge/filtered") - only if fog nodes exist
    if (has_fog) {
        for (auto* host : fog_nodes) {
            host->add_actor("fog_aggregator",
                            FogAggregator("edge/filtered", gateways.size() * sensors.size() * 5, 15.0));
        }
        XBT_INFO("MQTT Pipeline: Sensors -> Gateways -> Fog");
    } else {
        XBT_INFO("MQTT Pipeline: Sensors -> Gateways (edge-only)");
    }
    
    // Run simulation
    e.run();
    
    XBT_INFO("=== Simulation completed ===");
    XBT_INFO("Simulated time: %.2f seconds", sg4::Engine::get_clock());
    
    return 0;
}
