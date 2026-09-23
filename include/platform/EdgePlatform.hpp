#ifndef ENIGMA_EDGE_PLATFORM_HPP
#define ENIGMA_EDGE_PLATFORM_HPP

#include "platform/PlatformGenerator.hpp"
#include <string>

namespace enigma {

/**
 * @brief Generator for Edge Platforms
 *
 * Edge platforms have:
 * - Devices with limited resources
 * - Low latency between nearby devices
 * - High latency to external services
 * - Star or mesh topology
 */
class EdgePlatform {
public:
    /**
     * @brief Create an Edge platform with a central gateway
     */
    static ZoneConfig createStarTopology(int numDevices, 
                                          const std::string& deviceSpeed = "1Gf",
                                          const std::string& gatewaySpeed = "5Gf");
    
    /**
     * @brief Create an Edge platform with a mesh topology
     */
    static ZoneConfig createMeshTopology(int numDevices,
                                          const std::string& deviceSpeed = "1Gf");
    
    /**
     * @brief Create an Edge IoT platform with sensors and actuators
     */
    static ZoneConfig createIoTPlatform(int numSensors, int numActuators,
                                         const std::string& gatewaySpeed = "3Gf");
    
    /**
     * @brief Settings for Edge Devices
     */
    static HostConfig createEdgeDevice(const std::string& id, 
                                        const std::string& deviceType = "standard");
    
    /**
     * @brief Settings for Edge Links
     */
    static LinkConfig createEdgeLink(const std::string& id,
                                      const std::string& linkType = "wifi");
};

} // namespace enigma

#endif // ENIGMA_EDGE_PLATFORM_HPP
