#include <simgrid/s4u.hpp>
#include <iostream>
#include <vector>

XBT_LOG_NEW_DEFAULT_CATEGORY(hybrid_cloud, "Hybrid Cloud Application");

namespace sg4 = simgrid::s4u;

/**
 * @brief Hybrid Edge-Fog-Cloud application
 * 
 * Simulates a three-tier architecture where:
 * - Edge: Collects and filters data
 * - Fog: Aggregates and pre-processes
 * - Cloud: Stores and performs heavy analysis
 */

class EdgeCollector {
    std::string fog_destination;
    
public:
    EdgeCollector(const std::string& fog) : fog_destination(fog) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("[EDGE] '%s' started", this_host->get_cname());
        
        // Collect data
        for (int i = 0; i < 3; i++) {
            sg4::this_actor::sleep_for(1.0);
            
            // Local filtering
            XBT_INFO("[EDGE] Filtering data locally...");
            sg4::this_actor::execute(2e8);  // 200 MFlops
            
            // Send to Fog
            auto* mbox = sg4::Mailbox::by_name(fog_destination);
            auto* data = new std::string("edge_data_" + std::to_string(i));
            
            XBT_INFO("[EDGE] Sending data to Fog '%s'", fog_destination.c_str());
            mbox->put(data, 5e5);  // 500 KB
        }
        
        XBT_INFO("[EDGE] Collection completed");
    }
};

class FogAggregator {
    std::string cloud_destination;
    int num_edge_devices;
    
public:
    FogAggregator(const std::string& cloud, int num_devices)
        : cloud_destination(cloud), num_edge_devices(num_devices) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("[FOG] '%s' started", this_host->get_cname());
        
        auto* mbox = sg4::Mailbox::by_name(this_host->get_cname());
        std::vector<std::string> aggregated_data;
        
        // Receive data from Edge devices
        int messages_received = 0;
        int expected_messages = num_edge_devices * 3;  // 3 messages per device
        
        while (messages_received < expected_messages) {
            auto* data = mbox->get<std::string>();
            aggregated_data.push_back(*data);
            delete data;
            messages_received++;
            
            XBT_INFO("[FOG] Data received (%d/%d)", messages_received, expected_messages);
            
            // Pre-processing
            sg4::this_actor::execute(5e8);  // 500 MFlops
        }
        
        // Final aggregation
        XBT_INFO("[FOG] Performing aggregation of %zu data sets...",
                 aggregated_data.size());
        sg4::this_actor::execute(2e9);  // 2 GFlops
        
        // Send to Cloud
        auto* cloud_mbox = sg4::Mailbox::by_name(cloud_destination);
        auto* summary = new std::string("fog_summary_" + 
                                        std::to_string(aggregated_data.size()));
        
        XBT_INFO("[FOG] Sending summary to Cloud '%s'", cloud_destination.c_str());
        cloud_mbox->put(summary, 1e6);  // 1 MB
        
        XBT_INFO("[FOG] Processing completed");
    }
};

class CloudProcessor {
    int num_fog_nodes;
    
public:
    CloudProcessor(int num_fog) : num_fog_nodes(num_fog) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("[CLOUD] '%s' started", this_host->get_cname());
        
        auto* mbox = sg4::Mailbox::by_name(this_host->get_cname());
        
        // Receive summaries from Fog nodes
        for (int i = 0; i < num_fog_nodes; i++) {
            auto* summary = mbox->get<std::string>();
            
            XBT_INFO("[CLOUD] Received summary: %s", summary->c_str());
            delete summary;
            
            // Storage
            sg4::this_actor::execute(1e9);  // 1 GFlop
        }
        
        // Final heavy analysis
        XBT_INFO("[CLOUD] Performing complete analysis and machine learning...");
        sg4::this_actor::execute(10e9);  // 10 GFlops
        
        XBT_INFO("[CLOUD] Analysis completed and data stored");
        XBT_INFO("=== Edge-Fog-Cloud pipeline completed successfully ===");
    }
};

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform_file.xml>", argv[0]);
        XBT_CRITICAL("Example: %s platforms/hybrid_platform.xml", argv[0]);
        return 1;
    }
    
    e.load_platform(argv[1]);
    
    std::vector<sg4::Host*> hosts = e.get_all_hosts();
    
    if (hosts.size() < 3) {
        XBT_CRITICAL("At least 3 hosts required (Edge, Fog, Cloud)");
        return 1;
    }
    
    XBT_INFO("=== Hybrid Edge-Fog-Cloud Application ===");
    XBT_INFO("Platform loaded with %zu hosts", hosts.size());
    
    // Classify hosts by name or position
    std::vector<sg4::Host*> edge_hosts, fog_hosts, cloud_hosts;
    
    for (auto* host : hosts) {
        std::string name = host->get_cname();
        if (name.find("edge") != std::string::npos) {
            edge_hosts.push_back(host);
        } else if (name.find("fog") != std::string::npos) {
            fog_hosts.push_back(host);
        } else if (name.find("cloud") != std::string::npos) {
            cloud_hosts.push_back(host);
        }
    }
    
    // If no classification by name, divide into thirds
    if (edge_hosts.empty() && fog_hosts.empty() && cloud_hosts.empty()) {
        size_t third = hosts.size() / 3;
        edge_hosts.assign(hosts.begin(), hosts.begin() + third);
        fog_hosts.assign(hosts.begin() + third, hosts.begin() + 2 * third);
        cloud_hosts.assign(hosts.begin() + 2 * third, hosts.end());
    }
    
    // Ensure at least one host of each type
    if (edge_hosts.empty()) edge_hosts.push_back(hosts[0]);
    if (fog_hosts.empty()) fog_hosts.push_back(hosts[std::min(1UL, hosts.size()-1)]);
    if (cloud_hosts.empty()) cloud_hosts.push_back(hosts.back());
    
    XBT_INFO("Distribution:");
    XBT_INFO("  Edge: %zu hosts", edge_hosts.size());
    XBT_INFO("  Fog: %zu hosts", fog_hosts.size());
    XBT_INFO("  Cloud: %zu hosts", cloud_hosts.size());
    
    // Create Cloud actors
    for (auto* cloud : cloud_hosts) {
        cloud->add_actor("cloud_processor", CloudProcessor(fog_hosts.size()));
    }
    
    // Create Fog actors
    for (auto* fog : fog_hosts) {
        int devices_per_fog = edge_hosts.size() / fog_hosts.size();
        if (devices_per_fog == 0) devices_per_fog = edge_hosts.size();
        
        fog->add_actor("fog_aggregator",
                       FogAggregator(cloud_hosts[0]->get_cname(), devices_per_fog));
    }
    
    // Create Edge actors
    for (size_t i = 0; i < edge_hosts.size(); i++) {
        std::string fog_dest = fog_hosts[i % fog_hosts.size()]->get_cname();
        edge_hosts[i]->add_actor("edge_collector", EdgeCollector(fog_dest));
    }
    
    // Run simulation
    e.run();
    
    XBT_INFO("=== Simulation completed ===");
    XBT_INFO("Simulated time: %.2f seconds", sg4::Engine::get_clock());
    
    return 0;
}
