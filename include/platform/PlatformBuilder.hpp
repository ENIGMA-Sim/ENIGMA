#ifndef ENIGMA_PLATFORM_BUILDER_HPP
#define ENIGMA_PLATFORM_BUILDER_HPP

#include <string>
#include <vector>
#include <memory>
#include "platform/PlatformGenerator.hpp"

namespace enigma {

/**
 * @brief Builder para crear plataformas de forma fluida
 */
class PlatformBuilder {
public:
    PlatformBuilder();
    ~PlatformBuilder() = default;

    // Inicialización
    PlatformBuilder& createPlatform(const std::string& name);
    PlatformBuilder& createEdgeFogCloud(const std::string& name);
    
    // Construcción por capas
    PlatformBuilder& addEdgeLayer(int numDevices, const std::string& speed = "1Gf", 
                                   const std::string& bandwidth = "125MBps");
    PlatformBuilder& addFogLayer(int numNodes, const std::string& speed = "10Gf", 
                                  const std::string& bandwidth = "1GBps");
    PlatformBuilder& addCloudLayer(int numServers, const std::string& speed = "100Gf", 
                                    const std::string& bandwidth = "10GBps");
    
    // Construcción personalizada
    PlatformBuilder& addZone(const std::string& id, const std::string& routing = "Full");
    PlatformBuilder& addHost(const std::string& id, const std::string& speed, int cores = 1);
    PlatformBuilder& addLink(const std::string& id, const std::string& bandwidth, 
                              const std::string& latency = "50us");
    PlatformBuilder& addRoute(const std::string& src, const std::string& dst, 
                               const std::vector<std::string>& links);
    
    // Configuración avanzada
    PlatformBuilder& setRouting(const std::string& routing);
    PlatformBuilder& setLatency(const std::string& defaultLatency);
    PlatformBuilder& enableLoopback(bool enable = true);
    
    // Generación
    void build();
    void buildToFile(const std::string& filename);
    std::string getPlatformXML() const;

private:
    std::string platformName_;
    ZoneConfig rootZone_;
    std::vector<ZoneConfig> zones_;
    ZoneConfig* currentZone_;
    
    std::string defaultLatency_;
    bool loopbackEnabled_;
    
    PlatformGenerator generator_;
    
    void validateConfiguration() const;
};

} // namespace enigma

#endif // ENIGMA_PLATFORM_BUILDER_HPP
