#ifndef ENIGMA_PLATFORM_GENERATOR_HPP
#define ENIGMA_PLATFORM_GENERATOR_HPP

#include <string>
#include <vector>
#include <memory>
#include "utils/XMLWriter.hpp"

namespace enigma {

/**
 * @brief Infrastructure types
 */
enum class InfrastructureType {
    EDGE,
    FOG,
    CLOUD,
    HYBRID
};

/**
 * @brief Cluster configuration
 */
struct ClusterConfig {
    std::string id;
    int num_nodes;          // Number of nodes in the cluster
    std::string node_speed; // CPU speed per node (e.g., "1Gf", "10Gf")
    int cores_per_node;     // Cores per node
    std::string bandwidth;  // Internal cluster bandwidth
    std::string latency;    // Internal cluster latency
    std::string backbone_bw; // Backbone bandwidth (for cluster interconnection)
    std::string backbone_lat; // Backbone latency
    
    ClusterConfig(const std::string& id_, int nodes, const std::string& speed,
                  int cores = 1, const std::string& bw = "125MBps", 
                  const std::string& lat = "50us")
        : id(id_), num_nodes(nodes), node_speed(speed), cores_per_node(cores),
          bandwidth(bw), latency(lat), backbone_bw("1GBps"), backbone_lat("10us") {}
};

/**
 * @brief Host configuration
 */
struct HostConfig {
    std::string id;
    std::string speed;      // CPU speed (e.g., "1Gf", "10Gf")
    int core_count;
    std::string coordinates; // Optional coordinates
    
    HostConfig(const std::string& id_, const std::string& speed_, int cores = 1)
        : id(id_), speed(speed_), core_count(cores), coordinates("") {}
};

/**
 * @brief Link configuration
 */
struct LinkConfig {
    std::string id;
    std::string bandwidth;  // Bandwidth (e.g., "125MBps", "1GBps")
    std::string latency;    // Latency (e.g., "50us", "10ms")
    std::string sharing_policy; // "SHARED" or "FATPIPE"
    
    LinkConfig(const std::string& id_, const std::string& bw, const std::string& lat = "50us")
        : id(id_), bandwidth(bw), latency(lat), sharing_policy("SHARED") {}
};

/**
 * @brief Zone configuration
 */
struct ZoneConfig {
    std::string id;
    std::string routing;    // "Full", "Floyd", "Dijkstra", "Cluster", etc.
    std::vector<HostConfig> hosts;
    std::vector<LinkConfig> links;
    std::vector<ClusterConfig> clusters;  // Clusters in this zone
    std::vector<ZoneConfig> subzones;
    bool auto_interconnect; // Automatically create routes between all elements
    bool use_native_clusters; // Use SimGrid native <cluster> tags instead of expanding to hosts
    bool allow_direct_edge_cloud; // Flat hybrid: allow direct Edge <-> Cloud connectivity (skip Fog)
    bool force_flat_layout; // Treat as flat hybrid even if some tiers missing (avoid generic mesh links)
    
    ZoneConfig(const std::string& id_, const std::string& routing_ = "Full")
        : id(id_), routing(routing_), auto_interconnect(true), use_native_clusters(true), allow_direct_edge_cloud(false), force_flat_layout(false) {}
};

/**
 * @brief Generador de plataformas SimGrid
 */
class PlatformGenerator {
public:
    PlatformGenerator();
    virtual ~PlatformGenerator() = default;

    // Main methods
    void generatePlatform(const std::string& filename, const ZoneConfig& config);
    
    // Helpers to create typical configurations
    // Simple hosts (all interconnected)
    static ZoneConfig createEdgeZone(const std::string& id, int numDevices);
    static ZoneConfig createFogZone(const std::string& id, int numNodes);
    static ZoneConfig createCloudZone(const std::string& id, int numServers);
    // Keeping signature for backward compatibility; returns flat hybrid layout.
    static ZoneConfig createHybridPlatform(int edgeDevices, int fogNodes, int cloudServers);
    
    // Cluster-based configurations
    static ZoneConfig createEdgeWithClusters(const std::string& id, 
                                             const std::vector<ClusterConfig>& clusters);
    static ZoneConfig createFogWithClusters(const std::string& id,
                                            const std::vector<ClusterConfig>& clusters);
    static ZoneConfig createCloudWithClusters(const std::string& id,
                                              const std::vector<ClusterConfig>& clusters);
    
    // Hybrid with clusters
    // Flat hybrid (all clusters at same level) – optional direct Edge<->Cloud links.
    static ZoneConfig createHybridWithClustersFlat(
        const std::vector<ClusterConfig>& edgeClusters,
        const std::vector<ClusterConfig>& fogClusters,
        const std::vector<ClusterConfig>& cloudClusters,
        bool edgeCloudDirect = false);

protected:
    void writeZone(XMLWriter& writer, const ZoneConfig& zone, bool /* isRoot */);
    void writeHost(XMLWriter& writer, const HostConfig& host);
    void writeLink(XMLWriter& writer, const LinkConfig& link);
    void writeCluster(XMLWriter& writer, const ClusterConfig& cluster);
    void writeClusterAsHosts(XMLWriter& writer, const ClusterConfig& cluster);
    void writeRoute(XMLWriter& writer, const std::string& src, const std::string& dst, 
                    const std::vector<std::string>& links);
    
    // Utilities to generate automatic routes
    void generateFullRoutes(XMLWriter& writer, const ZoneConfig& zone);
    void generateFullRoutesWithClusters(XMLWriter& writer, const ZoneConfig& zone);
    void generateClusterInterconnection(XMLWriter& writer, const ZoneConfig& zone);
    void generateInterZoneRoutes(XMLWriter& writer, const ZoneConfig& zone);
    void generateFlatHybridRoutes(XMLWriter& writer, const ZoneConfig& zone);
};

} // namespace enigma

#endif // ENIGMA_PLATFORM_GENERATOR_HPP
