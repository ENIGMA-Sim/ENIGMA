#ifndef ENIGMA_CLOUD_PLATFORM_HPP
#define ENIGMA_CLOUD_PLATFORM_HPP

#include "platform/PlatformGenerator.hpp"
#include <string>

namespace enigma {

/**
 * @brief Generador especializado para plataformas Cloud
 * 
 * Las plataformas Cloud tienen:
 * - Servidores con alta capacidad de cómputo
 * - Alto ancho de banda entre servidores
 * - Topología de data center
 * - Clusters y racks
 */
class CloudPlatform {
public:
    /**
     * @brief Crea un data center con topología fat-tree
     */
    static ZoneConfig createDataCenter(int numRacks, int serversPerRack,
                                        const std::string& serverSpeed = "100Gf");
    
    /**
     * @brief Crea un cluster de servidores homogéneos
     */
    static ZoneConfig createCluster(int numServers,
                                     const std::string& serverSpeed = "100Gf",
                                     const std::string& interconnect = "10GBps");
    
    /**
     * @brief Crea una plataforma multi-cloud
     */
    static ZoneConfig createMultiCloud(int numClouds, int serversPerCloud,
                                        const std::string& serverSpeed = "100Gf");
    
    /**
     * @brief Crea un cluster con diferentes tipos de nodos (CPU, GPU)
     */
    static ZoneConfig createHeterogeneousCluster(int numCPUNodes, int numGPUNodes,
                                                   const std::string& cpuSpeed = "100Gf",
                                                   const std::string& gpuSpeed = "500Gf");
    
    /**
     * @brief Configuración típica para servidores Cloud
     */
    static HostConfig createCloudServer(const std::string& id,
                                         const std::string& serverType = "standard");
    
    /**
     * @brief Configuración típica para enlaces Cloud
     */
    static LinkConfig createCloudLink(const std::string& id,
                                       const std::string& linkType = "10G");
};

} // namespace enigma

#endif // ENIGMA_CLOUD_PLATFORM_HPP
