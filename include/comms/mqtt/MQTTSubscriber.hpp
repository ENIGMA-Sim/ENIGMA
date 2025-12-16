#ifndef ENIGMA_MQTT_SUBSCRIBER_HPP
#define ENIGMA_MQTT_SUBSCRIBER_HPP

#include "MQTTBroker.hpp"
#include <simgrid/s4u.hpp>
#include <string>
#include <vector>
#include <functional>

namespace enigma {
namespace mqtt {

/**
 * @brief MQTT Subscriber - Subscribes to topics and receives messages
 * 
 * Utility class to simplify subscribing to topics and receiving messages.
 */
class MQTTSubscriber {
private:
    std::string broker_name;
    std::string subscriber_id;
    simgrid::s4u::Mailbox* broker_mbox;
    simgrid::s4u::Mailbox* my_mbox;
    std::vector<std::string> subscribed_topics;
    
public:
    /**
     * @brief Construct MQTT Subscriber
     * @param broker Broker name to connect to
     * @param subscriber_id Unique identifier for this subscriber
     */
    MQTTSubscriber(const std::string& broker = "mqtt_broker",
                   const std::string& sub_id = "");
    
    /**
     * @brief Subscribe to a topic
     * @param topic Topic pattern (e.g., "sensors/temperature", "edge/#")
     */
    void subscribe(const std::string& topic);
    
    /**
     * @brief Unsubscribe from a topic
     * @param topic Topic to unsubscribe from
     */
    void unsubscribe(const std::string& topic);
    
    /**
     * @brief Receive next message (blocking)
     * @param timeout Maximum time to wait (-1 for infinite)
     * @return Received message or nullptr if timeout
     */
    std::shared_ptr<MQTTMessage> receive(double timeout = -1);
    
    /**
     * @brief Check if messages are available
     * @return True if messages are ready
     */
    bool has_messages() const;
    
    /**
     * @brief Get subscriber mailbox name
     */
    const std::string& get_mailbox_name() const;
    
    /**
     * @brief Get subscriber ID
     */
    const std::string& get_id() const { return subscriber_id; }
    
    /**
     * @brief Get list of subscribed topics
     */
    const std::vector<std::string>& get_topics() const { return subscribed_topics; }
};

} // namespace mqtt
} // namespace enigma

#endif // ENIGMA_MQTT_SUBSCRIBER_HPP
