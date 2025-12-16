#ifndef ENIGMA_FOG_PLATFORM_HPP
#define ENIGMA_FOG_PLATFORM_HPP

#include "platform/PlatformGenerator.hpp"
#include <string>

namespace enigma {

/**
 * @brief Generador especializado para plataformas Fog
 * 
 * Las plataformas Fog tienen:
 * - Nodos con capacidad media (más que Edge, menos que Cloud)
 * - Latencia media
 * - Conexiones a múltiples dispositivos Edge
 * - Topología jerárquica
 */
class FogPlatform {
public:
    /**
     * @brief Crea una plataforma Fog con topología jerárquica
     */
    static ZoneConfig createHierarchicalTopology(int numFogNodes,
                                                   const std::string& nodeSpeed = "10Gf");
    
    /**
     * @brief Crea una plataforma Fog conectada a dispositivos Edge
     */
    static ZoneConfig createEdgeFogTopology(int numFogNodes, int edgeDevicesPerNode,
                                             const std::string& fogSpeed = "10Gf",
                                             const std::string& edgeSpeed = "1Gf");
    
    /**
     * @brief Crea una plataforma Fog con nodos distribuidos geográficamente
     */
    static ZoneConfig createGeographicTopology(int numRegions, int nodesPerRegion,
                                                 const std::string& nodeSpeed = "10Gf");
    
    /**
     * @brief Configuración típica para nodos Fog
     */
    static HostConfig createFogNode(const std::string& id,
                                     const std::string& nodeType = "standard");
    
    /**
     * @brief Configuración típica para enlaces Fog
     */
    static LinkConfig createFogLink(const std::string& id,
                                     const std::string& linkType = "ethernet");
};

} // namespace enigma

#endif // ENIGMA_FOG_PLATFORM_HPP
