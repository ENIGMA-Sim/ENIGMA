#include "comms/mqtt/MQTTBroker.hpp"
#include <algorithm>

XBT_LOG_NEW_DEFAULT_CATEGORY(mqtt_broker, "MQTT Broker");

namespace enigma {
namespace mqtt {

namespace sg4 = simgrid::s4u;

MQTTBroker::MQTTBroker(const std::string& name)
    : broker_name(name), messages_published(0), messages_delivered(0), running(true) {
    control_mbox = sg4::Mailbox::by_name(get_broker_mailbox(name));
}

void MQTTBroker::operator()() {
    sg4::Host* host = sg4::this_actor::get_host();
    XBT_INFO("MQTT Broker '%s' started on host '%s'", 
             broker_name.c_str(), host->get_cname());
    
    while (running) {
        // Receive control message
        auto* ctrl_msg = control_mbox->get<MQTTControlMessage>();
        
        switch (ctrl_msg->type) {
            case MQTTControlMessage::Type::SUBSCRIBE:
                handle_subscribe(ctrl_msg->topic, ctrl_msg->subscriber);
                break;
                
            case MQTTControlMessage::Type::UNSUBSCRIBE:
                handle_unsubscribe(ctrl_msg->topic, ctrl_msg->subscriber);
                break;
                
            case MQTTControlMessage::Type::PUBLISH:
                handle_publish(ctrl_msg->message);
                break;
                
            case MQTTControlMessage::Type::SHUTDOWN:
                XBT_INFO("Broker shutdown requested");
                running = false;
                break;
                
            default:
                XBT_WARN("Unknown control message type");
                break;
        }
        
        delete ctrl_msg;
    }
    
    print_stats();
    XBT_INFO("MQTT Broker '%s' terminated", broker_name.c_str());
}

void MQTTBroker::handle_subscribe(const std::string& topic, const std::string& subscriber) {
    auto& subs = subscriptions[topic];
    
    // Check if already subscribed
    if (std::find(subs.begin(), subs.end(), subscriber) == subs.end()) {
        subs.push_back(subscriber);
        XBT_INFO("Subscriber '%s' subscribed to topic '%s' (%zu total subscribers)",
                 subscriber.c_str(), topic.c_str(), subs.size());
    } else {
        XBT_DEBUG("Subscriber '%s' already subscribed to topic '%s'",
                  subscriber.c_str(), topic.c_str());
    }
}

void MQTTBroker::handle_unsubscribe(const std::string& topic, const std::string& subscriber) {
    auto it = subscriptions.find(topic);
    if (it != subscriptions.end()) {
        auto& subs = it->second;
        auto sub_it = std::find(subs.begin(), subs.end(), subscriber);
        
        if (sub_it != subs.end()) {
            subs.erase(sub_it);
            XBT_INFO("Subscriber '%s' unsubscribed from topic '%s'",
                     subscriber.c_str(), topic.c_str());
            
            // Remove topic if no more subscribers
            if (subs.empty()) {
                subscriptions.erase(it);
                XBT_DEBUG("Topic '%s' removed (no subscribers)", topic.c_str());
            }
        }
    }
}

void MQTTBroker::handle_publish(std::shared_ptr<MQTTMessage> msg) {
    messages_published++;
    
    XBT_INFO("Publishing message to topic '%s' (size: %zu bytes, from: %s)",
             msg->topic.c_str(), msg->size, msg->publisher.c_str());
    
    // Find subscribers for this topic
    auto it = subscriptions.find(msg->topic);
    if (it == subscriptions.end() || it->second.empty()) {
        XBT_DEBUG("No subscribers for topic '%s'", msg->topic.c_str());
        return;
    }
    
    // Deliver to all subscribers
    for (const auto& subscriber : it->second) {
        auto* mbox = sg4::Mailbox::by_name(subscriber);
        
        // Create a copy of the message for each subscriber
        auto* msg_copy = new std::shared_ptr<MQTTMessage>(msg);
        
        XBT_DEBUG("Delivering message to subscriber '%s'", subscriber.c_str());
        mbox->put(msg_copy, msg->size);
        messages_delivered++;
    }
    
    XBT_INFO("Message delivered to %zu subscribers", it->second.size());
}

std::string MQTTBroker::get_broker_mailbox(const std::string& broker_name) {
    return "mqtt_broker_" + broker_name;
}

std::string MQTTBroker::get_topic_mailbox(const std::string& broker_name,
                                          const std::string& topic) {
    return "mqtt_" + broker_name + "_topic_" + topic;
}

void MQTTBroker::print_stats() const {
    XBT_INFO("=== MQTT Broker Statistics ===");
    XBT_INFO("  Messages published: %d", messages_published);
    XBT_INFO("  Messages delivered: %d", messages_delivered);
    XBT_INFO("  Active topics: %zu", subscriptions.size());
    
    for (const auto& [topic, subs] : subscriptions) {
        XBT_INFO("    Topic '%s': %zu subscribers", topic.c_str(), subs.size());
    }
}

} // namespace mqtt
} // namespace enigma
