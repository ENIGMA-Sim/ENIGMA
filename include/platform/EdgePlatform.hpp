#ifndef ENIGMA_EDGE_PLATFORM_HPP
#define ENIGMA_EDGE_PLATFORM_HPP

#include "platform/PlatformGenerator.hpp"
#include <string>

namespace enigma {

/**
 * @brief Generador especializado para plataformas Edge
 * 
 * Las plataformas Edge tienen:
 * - Dispositivos con recursos limitados
 * - Baja latencia entre dispositivos cercanos
 * - Alta latencia hacia servicios externos
 * - Topología estrella o malla
 */
class EdgePlatform {
public:
    /**
     * @brief Crea una plataforma Edge con gateway central
     */
    static ZoneConfig createStarTopology(int numDevices, 
                                          const std::string& deviceSpeed = "1Gf",
                                          const std::string& gatewaySpeed = "5Gf");
    
    /**
     * @brief Crea una plataforma Edge con topología mesh
     */
    static ZoneConfig createMeshTopology(int numDevices,
                                          const std::string& deviceSpeed = "1Gf");
    
    /**
     * @brief Crea una plataforma Edge IoT con sensores y actuadores
     */
    static ZoneConfig createIoTPlatform(int numSensors, int numActuators,
                                         const std::string& gatewaySpeed = "3Gf");
    
    /**
     * @brief Configuración típica para dispositivos Edge
     */
    static HostConfig createEdgeDevice(const std::string& id, 
                                        const std::string& deviceType = "standard");
    
    /**
     * @brief Configuración típica para enlaces Edge
     */
    static LinkConfig createEdgeLink(const std::string& id,
                                      const std::string& linkType = "wifi");
};

} // namespace enigma

#endif // ENIGMA_EDGE_PLATFORM_HPP
