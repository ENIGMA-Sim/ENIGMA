#include "platform/FogPlatform.hpp"
#include <sstream>

namespace enigma {

ZoneConfig FogPlatform::createHierarchicalTopology(int numFogNodes,
                                                    const std::string& nodeSpeed) {
    ZoneConfig zone("fog_hierarchical", "Full");
    
    // Create fog nodes in hierarchy
    for (int i = 0; i < numFogNodes; ++i) {
        std::string id = "fog_node_" + std::to_string(i);
        zone.hosts.emplace_back(id, nodeSpeed, 8);
    }
    
    // Links between fog nodes (high capacity)
    for (int i = 0; i < numFogNodes; ++i) {
        for (int j = i + 1; j < numFogNodes; ++j) {
            std::string linkId = "fog_link_" + std::to_string(i) + "_" + std::to_string(j);
            zone.links.emplace_back(linkId, "1GBps", "5ms");
        }
    }
    
    return zone;
}

ZoneConfig FogPlatform::createEdgeFogTopology(int numFogNodes, int edgeDevicesPerNode,
                                               const std::string& fogSpeed,
                                               const std::string& edgeSpeed) {
    ZoneConfig zone("edge_fog_topology", "Full");
    
    // Create fog nodes
    for (int i = 0; i < numFogNodes; ++i) {
        std::string fogId = "fog_node_" + std::to_string(i);
        zone.hosts.emplace_back(fogId, fogSpeed, 8);
        
        // Create edge subzone for each fog node
        ZoneConfig edgeSubzone("edge_zone_" + std::to_string(i), "Full");
        
        for (int j = 0; j < edgeDevicesPerNode; ++j) {
            std::string edgeId = "edge_" + std::to_string(i) + "_" + std::to_string(j);
            edgeSubzone.hosts.emplace_back(edgeId, edgeSpeed, 1);
        }
        
        // Link between edge and fog
        edgeSubzone.links.emplace_back("edge_to_fog_link", "500MBps", "8ms");
        
        zone.subzones.push_back(edgeSubzone);
    }
    
    // Links between fog nodes
    zone.links.emplace_back("fog_interconnect", "1GBps", "3ms");
    
    return zone;
}

ZoneConfig FogPlatform::createGeographicTopology(int numRegions, int nodesPerRegion,
                                                  const std::string& nodeSpeed) {
    ZoneConfig zone("fog_geographic", "Full");
    
    // Create geographic regions
    for (int r = 0; r < numRegions; ++r) {
        ZoneConfig region("region_" + std::to_string(r), "Full");
        
        // Fog nodes in each region
        for (int n = 0; n < nodesPerRegion; ++n) {
            std::string id = "fog_r" + std::to_string(r) + "_n" + std::to_string(n);
            region.hosts.emplace_back(id, nodeSpeed, 8);
        }
        
        // Intra-region links (low latency)
        region.links.emplace_back("intra_region_link", "1GBps", "2ms");
        
        zone.subzones.push_back(region);
    }
    
    // Inter-region links (higher latency)
    zone.links.emplace_back("inter_region_link", "500MBps", "20ms");
    
    return zone;
}

HostConfig FogPlatform::createFogNode(const std::string& id,
                                      const std::string& nodeType) {
    if (nodeType == "lightweight") {
        return HostConfig(id, "5Gf", 4);
    } else if (nodeType == "standard") {
        return HostConfig(id, "10Gf", 8);
    } else if (nodeType == "powerful") {
        return HostConfig(id, "20Gf", 16);
    } else if (nodeType == "edge_server") {
        return HostConfig(id, "15Gf", 12);
    } else {
        return HostConfig(id, "10Gf", 8);
    }
}

LinkConfig FogPlatform::createFogLink(const std::string& id,
                                      const std::string& linkType) {
    if (linkType == "ethernet") {
        return LinkConfig(id, "1GBps", "1ms");
    } else if (linkType == "fiber") {
        return LinkConfig(id, "10GBps", "500us");
    } else if (linkType == "wireless_5g") {
        return LinkConfig(id, "2GBps", "3ms");
    } else if (linkType == "wan") {
        return LinkConfig(id, "500MBps", "15ms");
    } else {
        // Default ethernet
        return LinkConfig(id, "1GBps", "2ms");
    }
}

} // namespace enigma
