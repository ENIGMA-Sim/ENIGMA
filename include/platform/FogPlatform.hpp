#ifndef ENIGMA_FOG_PLATFORM_HPP
#define ENIGMA_FOG_PLATFORM_HPP

#include "platform/PlatformGenerator.hpp"
#include <string>

namespace enigma {

/**
 * @brief Fog Platform Generator
 *
 * Fog platforms have:
 * - Nodes with medium capacity (more than Edge, less than Cloud)
 * - Medium latency
 * - Connections to multiple Edge devices
 * - Hierarchical topology
 */
class FogPlatform {
public:
    /**
     * @brief Create a Fog platform with a hierarchical topology
     */
    static ZoneConfig createHierarchicalTopology(int numFogNodes,
                                                   const std::string& nodeSpeed = "10Gf");
    
    /**
     * @brief Create a Fog platform connected to edge devices
     */
    static ZoneConfig createEdgeFogTopology(int numFogNodes, int edgeDevicesPerNode,
                                             const std::string& fogSpeed = "10Gf",
                                             const std::string& edgeSpeed = "1Gf");
    
    /**
     * @brief Create a Fog platform with geographically distributed nodes
     */
    static ZoneConfig createGeographicTopology(int numRegions, int nodesPerRegion,
                                                 const std::string& nodeSpeed = "10Gf");
    
    /**
     * @brief Configuration for Fog Nodes
     */
    static HostConfig createFogNode(const std::string& id,
                                     const std::string& nodeType = "standard");
    
    /**
     * @brief Fog Link Settings
     */
    static LinkConfig createFogLink(const std::string& id,
                                     const std::string& linkType = "ethernet");
};

} // namespace enigma

#endif // ENIGMA_FOG_PLATFORM_HPP
