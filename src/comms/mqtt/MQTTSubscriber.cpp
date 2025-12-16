#include "comms/mqtt/MQTTSubscriber.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(mqtt_subscriber, "MQTT Subscriber");

namespace enigma {
namespace mqtt {

namespace sg4 = simgrid::s4u;

MQTTSubscriber::MQTTSubscriber(const std::string& broker, const std::string& sub_id)
    : broker_name(broker) {
    
    // Generate subscriber ID if not provided
    if (sub_id.empty()) {
        auto* host = sg4::this_actor::get_host();
        subscriber_id = std::string(host->get_cname()) + "_sub_" + 
                       std::to_string(sg4::this_actor::get_pid());
    } else {
        subscriber_id = sub_id;
    }
    
    broker_mbox = sg4::Mailbox::by_name(MQTTBroker::get_broker_mailbox(broker_name));
    my_mbox = sg4::Mailbox::by_name(subscriber_id);
    
    XBT_DEBUG("MQTT Subscriber '%s' initialized (broker: %s)",
              subscriber_id.c_str(), broker_name.c_str());
}

void MQTTSubscriber::subscribe(const std::string& topic) {
    auto* ctrl_msg = MQTTControlMessage::subscribe(topic, subscriber_id);
    
    XBT_DEBUG("Subscribing to topic '%s'", topic.c_str());
    
    broker_mbox->put(ctrl_msg, sizeof(MQTTControlMessage));
    subscribed_topics.push_back(topic);
}

void MQTTSubscriber::unsubscribe(const std::string& topic) {
    auto* ctrl_msg = MQTTControlMessage::unsubscribe(topic, subscriber_id);
    
    XBT_DEBUG("Unsubscribing from topic '%s'", topic.c_str());
    
    broker_mbox->put(ctrl_msg, sizeof(MQTTControlMessage));
    
    // Remove from local list
    auto it = std::find(subscribed_topics.begin(), subscribed_topics.end(), topic);
    if (it != subscribed_topics.end()) {
        subscribed_topics.erase(it);
    }
}

std::shared_ptr<MQTTMessage> MQTTSubscriber::receive(double timeout) {
    try {
        std::shared_ptr<MQTTMessage>* msg_ptr;
        
        if (timeout > 0) {
            msg_ptr = my_mbox->get<std::shared_ptr<MQTTMessage>>(timeout);
        } else {
            msg_ptr = my_mbox->get<std::shared_ptr<MQTTMessage>>();
        }
        
        auto msg = *msg_ptr;
        delete msg_ptr;
        
        XBT_DEBUG("Received message from topic '%s' (size: %zu bytes)",
                  msg->topic.c_str(), msg->size);
        
        return msg;
        
    } catch (const simgrid::TimeoutException&) {
        XBT_DEBUG("Receive timeout");
        return nullptr;
    }
}

bool MQTTSubscriber::has_messages() const {
    return my_mbox->ready();
}

const std::string& MQTTSubscriber::get_mailbox_name() const {
    return subscriber_id;
}

} // namespace mqtt
} // namespace enigma
