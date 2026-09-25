#ifndef ENIGMA_CLOUD_PLATFORM_HPP
#define ENIGMA_CLOUD_PLATFORM_HPP

#include "platform/PlatformGenerator.hpp"
#include <string>

namespace enigma {

/**
 * @brief Generator for Cloud Platforms
 *
 * Cloud platforms have:
 * - Servers with high computing capacity
 * - High bandwidth between servers
 * - Data center topology
 * - Clusters and racks
 */
class CloudPlatform {
public:
    /**
     * @brief Create a data center with a fat-tree topology
     */
    static ZoneConfig createDataCenter(int numRacks, int serversPerRack,
                                        const std::string& serverSpeed = "100Gf");
    
    /**
     * @brief Create a cluster of homogeneous servers
     */
    static ZoneConfig createCluster(int numServers,
                                     const std::string& serverSpeed = "100Gf",
                                     const std::string& interconnect = "10GBps");
    
    /**
     * @brief Create a multi-cloud platform
     */
    static ZoneConfig createMultiCloud(int numClouds, int serversPerCloud,
                                        const std::string& serverSpeed = "100Gf");
    
    /**
     * @brief Create a cluster with different types of nodes (CPU, GPU)
     */
    static ZoneConfig createHeterogeneousCluster(int numCPUNodes, int numGPUNodes,
                                                   const std::string& cpuSpeed = "100Gf",
                                                   const std::string& gpuSpeed = "500Gf");
    
    /**
     * @brief Configuration for Cloud Servers
     */
    static HostConfig createCloudServer(const std::string& id,
                                         const std::string& serverType = "standard");
    
    /**
     * @brief Cloud Link Settings
     */
    static LinkConfig createCloudLink(const std::string& id,
                                       const std::string& linkType = "10G");
};

} // namespace enigma

#endif // ENIGMA_CLOUD_PLATFORM_HPP
