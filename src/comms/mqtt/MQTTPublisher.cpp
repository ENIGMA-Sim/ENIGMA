#include "comms/mqtt/MQTTPublisher.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(mqtt_publisher, "MQTT Publisher");

namespace enigma {
namespace mqtt {

namespace sg4 = simgrid::s4u;

MQTTPublisher::MQTTPublisher(const std::string& broker, const std::string& pub_id)
    : broker_name(broker) {
    
    // Generate publisher ID if not provided
    if (pub_id.empty()) {
        auto* host = sg4::this_actor::get_host();
        publisher_id = std::string(host->get_cname()) + "_pub_" + 
                      std::to_string(sg4::this_actor::get_pid());
    } else {
        publisher_id = pub_id;
    }
    
    broker_mbox = sg4::Mailbox::by_name(MQTTBroker::get_broker_mailbox(broker_name));
    
    XBT_DEBUG("MQTT Publisher '%s' initialized (broker: %s)",
              publisher_id.c_str(), broker_name.c_str());
}

void MQTTPublisher::publish(const std::string& topic,
                            const std::string& payload,
                            size_t size,
                            int qos) {
    // Create MQTT message
    auto msg = std::make_shared<MQTTMessage>(topic, payload, size, publisher_id, qos);
    
    // Send to broker
    auto* ctrl_msg = MQTTControlMessage::publish(msg);
    
    XBT_DEBUG("Publishing to topic '%s' (size: %zu bytes, QoS: %d)",
              topic.c_str(), size, qos);
    
    broker_mbox->put(ctrl_msg, sizeof(MQTTControlMessage));
}

void MQTTPublisher::publish(const std::string& topic,
                            const std::string& payload,
                            int qos) {
    // Automatic size calculation
    size_t size = payload.size();
    publish(topic, payload, size, qos);
}

} // namespace mqtt
} // namespace enigma
