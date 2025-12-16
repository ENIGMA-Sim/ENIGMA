#ifndef ENIGMA_MQTT_HPP
#define ENIGMA_MQTT_HPP

/**
 * @file MQTT.hpp
 * @brief Convenience header for MQTT functionality
 * 
 * Include this file to access all MQTT components:
 * - MQTTBroker: Central message broker
 * - MQTTPublisher: Publish messages to topics
 * - MQTTSubscriber: Subscribe and receive messages
 */

#include "comms/mqtt/MQTTBroker.hpp"
#include "comms/mqtt/MQTTPublisher.hpp"
#include "comms/mqtt/MQTTSubscriber.hpp"

namespace enigma {
namespace mqtt {

/**
 * @brief MQTT Configuration options
 */
struct MQTTConfig {
    bool enabled;              // Enable/disable MQTT
    std::string broker_name;   // Broker identifier
    std::string broker_host;   // Host to run broker on
    bool auto_start_broker;    // Automatically start broker
    int default_qos;           // Default Quality of Service
    
    MQTTConfig()
        : enabled(false),
          broker_name("mqtt_broker"),
          broker_host(""),
          auto_start_broker(true),
          default_qos(0) {}
    
    static MQTTConfig create_disabled() {
        MQTTConfig config;
        config.enabled = false;
        return config;
    }
    
    static MQTTConfig create_enabled(const std::string& broker_host_name = "") {
        MQTTConfig config;
        config.enabled = true;
        config.broker_host = broker_host_name;
        return config;
    }
};

/**
 * @brief Helper function to start MQTT broker on a host
 * @param host Host to run broker on
 * @param broker_name Broker identifier
 * @return Broker actor
 */
inline simgrid::s4u::ActorPtr start_broker(simgrid::s4u::Host* host,
                                           const std::string& broker_name = "mqtt_broker") {
    return host->add_actor("mqtt_broker", MQTTBroker(broker_name));
}

/**
 * @brief Helper function to create a publisher
 * @param broker_name Broker to connect to
 * @param publisher_id Publisher identifier (auto-generated if empty)
 * @return MQTT Publisher instance
 */
inline MQTTPublisher create_publisher(const std::string& broker_name = "mqtt_broker",
                                      const std::string& publisher_id = "") {
    return MQTTPublisher(broker_name, publisher_id);
}

/**
 * @brief Helper function to create a subscriber
 * @param broker_name Broker to connect to
 * @param subscriber_id Subscriber identifier (auto-generated if empty)
 * @return MQTT Subscriber instance
 */
inline MQTTSubscriber create_subscriber(const std::string& broker_name = "mqtt_broker",
                                        const std::string& subscriber_id = "") {
    return MQTTSubscriber(broker_name, subscriber_id);
}

} // namespace mqtt
} // namespace enigma

#endif // ENIGMA_MQTT_HPP
