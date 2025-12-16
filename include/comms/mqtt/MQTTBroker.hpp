#ifndef ENIGMA_MQTT_BROKER_HPP
#define ENIGMA_MQTT_BROKER_HPP

#include <simgrid/s4u.hpp>
#include <string>
#include <map>
#include <vector>
#include <memory>

namespace enigma {
namespace mqtt {

/**
 * @brief MQTT Message structure
 */
struct MQTTMessage {
    std::string topic;
    std::string payload;
    size_t size;  // Size in bytes
    double timestamp;
    std::string publisher;
    int qos;  // Quality of Service (0, 1, 2)
    
    MQTTMessage(const std::string& t, const std::string& p, size_t s, 
                const std::string& pub, int q = 0)
        : topic(t), payload(p), size(s), publisher(pub), qos(q) {
        timestamp = simgrid::s4u::Engine::get_clock();
    }
};

/**
 * @brief MQTT Broker - Central message broker for pub/sub
 * 
 * Manages topic subscriptions and message distribution.
 * Runs as a SimGrid actor on a designated host.
 */
class MQTTBroker {
private:
    std::string broker_name;
    simgrid::s4u::Mailbox* control_mbox;
    
    // Topic subscriptions: topic -> list of subscriber mailboxes
    std::map<std::string, std::vector<std::string>> subscriptions;
    
    // Statistics
    int messages_published;
    int messages_delivered;
    
    bool running;
    
public:
    /**
     * @brief Construct MQTT Broker
     * @param name Broker name (used for mailbox identification)
     */
    explicit MQTTBroker(const std::string& name = "mqtt_broker");
    
    /**
     * @brief Main broker loop - SimGrid actor operator
     */
    void operator()();
    
    /**
     * @brief Get broker mailbox name
     */
    static std::string get_broker_mailbox(const std::string& broker_name = "mqtt_broker");
    
    /**
     * @brief Get topic mailbox name
     */
    static std::string get_topic_mailbox(const std::string& broker_name,
                                         const std::string& topic);
    
    /**
     * @brief Get statistics
     */
    void print_stats() const;
    
private:
    void handle_subscribe(const std::string& topic, const std::string& subscriber);
    void handle_unsubscribe(const std::string& topic, const std::string& subscriber);
    void handle_publish(std::shared_ptr<MQTTMessage> msg);
};

/**
 * @brief MQTT Control Messages
 */
struct MQTTControlMessage {
    enum class Type {
        SUBSCRIBE,
        UNSUBSCRIBE,
        PUBLISH,
        DISCONNECT,
        SHUTDOWN
    };
    
    Type type;
    std::string topic;
    std::string subscriber;
    std::shared_ptr<MQTTMessage> message;
    
    MQTTControlMessage(Type t) : type(t) {}
    
    static MQTTControlMessage* subscribe(const std::string& topic, 
                                         const std::string& subscriber) {
        auto* msg = new MQTTControlMessage(Type::SUBSCRIBE);
        msg->topic = topic;
        msg->subscriber = subscriber;
        return msg;
    }
    
    static MQTTControlMessage* unsubscribe(const std::string& topic,
                                           const std::string& subscriber) {
        auto* msg = new MQTTControlMessage(Type::UNSUBSCRIBE);
        msg->topic = topic;
        msg->subscriber = subscriber;
        return msg;
    }
    
    static MQTTControlMessage* publish(std::shared_ptr<MQTTMessage> mqtt_msg) {
        auto* msg = new MQTTControlMessage(Type::PUBLISH);
        msg->message = mqtt_msg;
        return msg;
    }
    
    static MQTTControlMessage* shutdown() {
        return new MQTTControlMessage(Type::SHUTDOWN);
    }
};

} // namespace mqtt
} // namespace enigma

#endif // ENIGMA_MQTT_BROKER_HPP
