#ifndef ENIGMA_MQTT_PUBLISHER_HPP
#define ENIGMA_MQTT_PUBLISHER_HPP

#include "MQTTBroker.hpp"
#include <simgrid/s4u.hpp>
#include <string>

namespace enigma {
namespace mqtt {

/**
 * @brief MQTT Publisher - Publishes messages to topics
 * 
 * Utility class to simplify publishing messages through MQTT broker.
 */
class MQTTPublisher {
private:
    std::string broker_name;
    std::string publisher_id;
    simgrid::s4u::Mailbox* broker_mbox;
    
public:
    /**
     * @brief Construct MQTT Publisher
     * @param broker Broker name to connect to
     * @param publisher_id Unique identifier for this publisher
     */
    MQTTPublisher(const std::string& broker = "mqtt_broker",
                  const std::string& pub_id = "");
    
    /**
     * @brief Publish a message to a topic
     * @param topic Topic name (e.g., "sensors/temperature", "edge/data")
     * @param payload Message content
     * @param size Message size in bytes
     * @param qos Quality of Service (0=at most once, 1=at least once, 2=exactly once)
     */
    void publish(const std::string& topic,
                 const std::string& payload,
                 size_t size,
                 int qos = 0);
    
    /**
     * @brief Publish with automatic size calculation
     */
    void publish(const std::string& topic,
                 const std::string& payload,
                 int qos = 0);
    
    /**
     * @brief Get publisher ID
     */
    const std::string& get_id() const { return publisher_id; }
};

} // namespace mqtt
} // namespace enigma

#endif // ENIGMA_MQTT_PUBLISHER_HPP
