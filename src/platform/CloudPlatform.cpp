#include "platform/CloudPlatform.hpp"
#include <sstream>

namespace enigma {

ZoneConfig CloudPlatform::createDataCenter(int numRacks, int serversPerRack,
                                            const std::string& serverSpeed) {
    ZoneConfig zone("data_center", "Full");
    
    // Create racks
    for (int r = 0; r < numRacks; ++r) {
        ZoneConfig rack("rack_" + std::to_string(r), "Full");
        
        // Servers in each rack
        for (int s = 0; s < serversPerRack; ++s) {
            std::string id = "server_r" + std::to_string(r) + "_s" + std::to_string(s);
            rack.hosts.emplace_back(id, serverSpeed, 32);
        }
        
        // Intra-rack link (very high speed)
        rack.links.emplace_back("rack_switch", "40GBps", "10us");
        
        zone.subzones.push_back(rack);
    }
    
    // Inter-rack links (high speed, higher latency)
    zone.links.emplace_back("spine_switch", "10GBps", "50us");
    
    return zone;
}

ZoneConfig CloudPlatform::createCluster(int numServers,
                                        const std::string& serverSpeed,
                                        const std::string& interconnect) {
    ZoneConfig zone("cloud_cluster", "Full");
    
    // Create homogeneous servers
    for (int i = 0; i < numServers; ++i) {
        std::string id = "server_" + std::to_string(i);
        zone.hosts.emplace_back(id, serverSpeed, 32);
    }
    
    // High-speed interconnect
    zone.links.emplace_back("cluster_interconnect", interconnect, "100us");
    
    return zone;
}

ZoneConfig CloudPlatform::createMultiCloud(int numClouds, int serversPerCloud,
                                           const std::string& serverSpeed) {
    ZoneConfig zone("multi_cloud", "Full");
    
    // Create multiple clouds
    for (int c = 0; c < numClouds; ++c) {
        ZoneConfig cloud("cloud_" + std::to_string(c), "Full");
        
        // Servers in each cloud
        for (int s = 0; s < serversPerCloud; ++s) {
            std::string id = "cloud" + std::to_string(c) + "_server_" + std::to_string(s);
            cloud.hosts.emplace_back(id, serverSpeed, 32);
        }
        
        // Intra-cloud link
        cloud.links.emplace_back("intra_cloud_link", "10GBps", "100us");
        
        zone.subzones.push_back(cloud);
    }
    
    // Inter-cloud links (WAN)
    zone.links.emplace_back("inter_cloud_link", "1GBps", "50ms");
    
    return zone;
}

ZoneConfig CloudPlatform::createHeterogeneousCluster(int numCPUNodes, int numGPUNodes,
                                                      const std::string& cpuSpeed,
                                                      const std::string& gpuSpeed) {
    ZoneConfig zone("heterogeneous_cluster", "Full");
    
    // CPU nodes
    for (int i = 0; i < numCPUNodes; ++i) {
        std::string id = "cpu_node_" + std::to_string(i);
        zone.hosts.emplace_back(id, cpuSpeed, 64);
    }
    
    // GPU nodes (higher compute capacity)
    for (int i = 0; i < numGPUNodes; ++i) {
        std::string id = "gpu_node_" + std::to_string(i);
        zone.hosts.emplace_back(id, gpuSpeed, 128);
    }
    
    // High-performance interconnect (InfiniBand)
    zone.links.emplace_back("infiniband", "100GBps", "1us");
    
    return zone;
}

HostConfig CloudPlatform::createCloudServer(const std::string& id,
                                            const std::string& serverType) {
    if (serverType == "small") {
        return HostConfig(id, "50Gf", 8);
    } else if (serverType == "medium") {
        return HostConfig(id, "100Gf", 16);
    } else if (serverType == "large") {
        return HostConfig(id, "200Gf", 32);
    } else if (serverType == "xlarge") {
        return HostConfig(id, "400Gf", 64);
    } else if (serverType == "gpu") {
        return HostConfig(id, "1000Gf", 128);
    } else if (serverType == "hpc") {
        return HostConfig(id, "500Gf", 128);
    } else {
        // Standard
        return HostConfig(id, "100Gf", 32);
    }
}

LinkConfig CloudPlatform::createCloudLink(const std::string& id,
                                          const std::string& linkType) {
    if (linkType == "1G") {
        return LinkConfig(id, "1GBps", "500us");
    } else if (linkType == "10G") {
        return LinkConfig(id, "10GBps", "100us");
    } else if (linkType == "40G") {
        return LinkConfig(id, "40GBps", "50us");
    } else if (linkType == "100G") {
        return LinkConfig(id, "100GBps", "10us");
    } else if (linkType == "infiniband") {
        return LinkConfig(id, "200GBps", "1us");
    } else {
        // Default 10G
        return LinkConfig(id, "10GBps", "100us");
    }
}

} // namespace enigma
