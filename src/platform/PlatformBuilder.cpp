#include "platform/PlatformBuilder.hpp"
#include <iostream>
#include <stdexcept>

namespace enigma {

PlatformBuilder::PlatformBuilder() 
    : platformName_("platform"),
      rootZone_("root", "Full"),
      currentZone_(&rootZone_),
      defaultLatency_("50us"),
      loopbackEnabled_(true) {
}

PlatformBuilder& PlatformBuilder::createPlatform(const std::string& name) {
    platformName_ = name;
    return *this;
}

PlatformBuilder& PlatformBuilder::createEdgeFogCloud(const std::string& name) {
    platformName_ = name;
    rootZone_.id = "hybrid_platform";
    return *this;
}

PlatformBuilder& PlatformBuilder::addEdgeLayer(int numDevices, 
                                                const std::string& speed,
                                                const std::string& bandwidth) {
    ZoneConfig edgeZone("edge_layer", "Full");
    
    for (int i = 0; i < numDevices; ++i) {
        std::string id = "edge_" + std::to_string(i);
        edgeZone.hosts.emplace_back(id, speed, 1);
    }
    
    edgeZone.links.emplace_back("edge_link", bandwidth, "5ms");
    rootZone_.subzones.push_back(edgeZone);
    
    return *this;
}

PlatformBuilder& PlatformBuilder::addFogLayer(int numNodes,
                                               const std::string& speed,
                                               const std::string& bandwidth) {
    ZoneConfig fogZone("fog_layer", "Full");
    
    for (int i = 0; i < numNodes; ++i) {
        std::string id = "fog_" + std::to_string(i);
        fogZone.hosts.emplace_back(id, speed, 4);
    }
    
    fogZone.links.emplace_back("fog_link", bandwidth, "2ms");
    rootZone_.subzones.push_back(fogZone);
    
    return *this;
}

PlatformBuilder& PlatformBuilder::addCloudLayer(int numServers,
                                                 const std::string& speed,
                                                 const std::string& bandwidth) {
    ZoneConfig cloudZone("cloud_layer", "Full");
    
    for (int i = 0; i < numServers; ++i) {
        std::string id = "cloud_" + std::to_string(i);
        cloudZone.hosts.emplace_back(id, speed, 16);
    }
    
    cloudZone.links.emplace_back("cloud_link", bandwidth, "100us");
    rootZone_.subzones.push_back(cloudZone);
    
    return *this;
}

PlatformBuilder& PlatformBuilder::addZone(const std::string& id, 
                                           const std::string& routing) {
    ZoneConfig zone(id, routing);
    zones_.push_back(zone);
    currentZone_ = &zones_.back();
    return *this;
}

PlatformBuilder& PlatformBuilder::addHost(const std::string& id, 
                                           const std::string& speed, 
                                           int cores) {
    if (currentZone_) {
        currentZone_->hosts.emplace_back(id, speed, cores);
    }
    return *this;
}

PlatformBuilder& PlatformBuilder::addLink(const std::string& id,
                                           const std::string& bandwidth,
                                           const std::string& latency) {
    if (currentZone_) {
        currentZone_->links.emplace_back(id, bandwidth, latency);
    }
    return *this;
}

PlatformBuilder& PlatformBuilder::addRoute(const std::string& /* src */,
                                            const std::string& /* dst */,
                                            const std::vector<std::string>& /* links */) {
    // Routes are generated automatically in Full routing
    // For custom routing, ZoneConfig would need to be extended
    return *this;
}

PlatformBuilder& PlatformBuilder::setRouting(const std::string& routing) {
    if (currentZone_) {
        currentZone_->routing = routing;
    }
    return *this;
}

PlatformBuilder& PlatformBuilder::setLatency(const std::string& defaultLatency) {
    defaultLatency_ = defaultLatency;
    return *this;
}

PlatformBuilder& PlatformBuilder::enableLoopback(bool enable) {
    loopbackEnabled_ = enable;
    return *this;
}

void PlatformBuilder::build() {
    std::string filename = "platforms/" + platformName_ + ".xml";
    buildToFile(filename);
}

void PlatformBuilder::buildToFile(const std::string& filename) {
    validateConfiguration();
    
    // Add custom zones to root zone
    for (const auto& zone : zones_) {
        rootZone_.subzones.push_back(zone);
    }
    
    generator_.generatePlatform(filename, rootZone_);
    
    std::cout << "Platform built successfully: " << filename << std::endl;
}

std::string PlatformBuilder::getPlatformXML() const {
    // Simplified implementation - return filename
    return platformName_ + ".xml";
}

void PlatformBuilder::validateConfiguration() const {
    if (rootZone_.hosts.empty() && rootZone_.subzones.empty()) {
        throw std::runtime_error("Platform contains no hosts or zones");
    }
    
    // Verify each zone has at least one host
    for (const auto& zone : rootZone_.subzones) {
        if (zone.hosts.empty()) {
            std::cerr << "Warning: Zone '" << zone.id 
                      << "' contains no hosts" << std::endl;
        }
    }
}

} // namespace enigma
