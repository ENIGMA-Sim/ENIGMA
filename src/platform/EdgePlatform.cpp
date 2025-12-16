#include "platform/EdgePlatform.hpp"
#include <sstream>

namespace enigma {

ZoneConfig EdgePlatform::createStarTopology(int numDevices,
                                             const std::string& deviceSpeed,
                                             const std::string& gatewaySpeed) {
    ZoneConfig zone("edge_star", "Full");
    
    // Central gateway
    zone.hosts.emplace_back("edge_gateway", gatewaySpeed, 2);
    
    // Edge devices
    for (int i = 0; i < numDevices; ++i) {
        std::string id = "edge_device_" + std::to_string(i);
        zone.hosts.emplace_back(id, deviceSpeed, 1);
        
        // Link between device and gateway
        std::string linkId = "link_device_" + std::to_string(i) + "_gateway";
        zone.links.emplace_back(linkId, "100MBps", "10ms");
    }
    
    return zone;
}

ZoneConfig EdgePlatform::createMeshTopology(int numDevices,
                                             const std::string& deviceSpeed) {
    ZoneConfig zone("edge_mesh", "Full");
    
    // Create devices
    for (int i = 0; i < numDevices; ++i) {
        std::string id = "edge_device_" + std::to_string(i);
        zone.hosts.emplace_back(id, deviceSpeed, 1);
    }
    
    // Create mesh links (all-to-all)
    for (int i = 0; i < numDevices; ++i) {
        for (int j = i + 1; j < numDevices; ++j) {
            std::string linkId = "link_" + std::to_string(i) + "_" + std::to_string(j);
            zone.links.emplace_back(linkId, "50MBps", "15ms");
        }
    }
    
    return zone;
}

ZoneConfig EdgePlatform::createIoTPlatform(int numSensors, int numActuators,
                                            const std::string& gatewaySpeed) {
    ZoneConfig zone("iot_platform", "Full");
    
    // IoT Gateway
    zone.hosts.emplace_back("iot_gateway", gatewaySpeed, 2);
    
    // Sensors (low-power devices)
    for (int i = 0; i < numSensors; ++i) {
        std::string id = "sensor_" + std::to_string(i);
        zone.hosts.emplace_back(id, "500Mf", 1);  // 500 MFlops
        
        std::string linkId = "link_sensor_" + std::to_string(i);
        zone.links.emplace_back(linkId, "10MBps", "20ms");  // WiFi/Zigbee
    }
    
    // Actuators
    for (int i = 0; i < numActuators; ++i) {
        std::string id = "actuator_" + std::to_string(i);
        zone.hosts.emplace_back(id, "800Mf", 1);  // 800 MFlops
        
        std::string linkId = "link_actuator_" + std::to_string(i);
        zone.links.emplace_back(linkId, "20MBps", "15ms");
    }
    
    return zone;
}

HostConfig EdgePlatform::createEdgeDevice(const std::string& id,
                                          const std::string& deviceType) {
    if (deviceType == "sensor") {
        return HostConfig(id, "500Mf", 1);
    } else if (deviceType == "smartphone") {
        return HostConfig(id, "2Gf", 4);
    } else if (deviceType == "raspberry_pi") {
        return HostConfig(id, "1.5Gf", 4);
    } else if (deviceType == "gateway") {
        return HostConfig(id, "5Gf", 2);
    } else {
        // Standard edge device
        return HostConfig(id, "1Gf", 1);
    }
}

LinkConfig EdgePlatform::createEdgeLink(const std::string& id,
                                        const std::string& linkType) {
    if (linkType == "wifi") {
        return LinkConfig(id, "54MBps", "10ms");
    } else if (linkType == "5g") {
        return LinkConfig(id, "1GBps", "5ms");
    } else if (linkType == "zigbee") {
        return LinkConfig(id, "250KBps", "20ms");
    } else if (linkType == "bluetooth") {
        return LinkConfig(id, "3MBps", "15ms");
    } else {
        // Default WiFi
        return LinkConfig(id, "100MBps", "10ms");
    }
}

} // namespace enigma
