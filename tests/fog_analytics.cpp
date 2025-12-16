#include <simgrid/s4u.hpp>
#include <iostream>
#include <vector>
#include <random>

XBT_LOG_NEW_DEFAULT_CATEGORY(fog_analytics, "Fog Analytics Application");

namespace sg4 = simgrid::s4u;

/**
 * @brief Fog data analytics application
 * 
 * Simulates a system where Edge devices send data to Fog nodes
 * for analysis and aggregation before sending to Cloud.
 */

// Actor for Edge device (data generator)
class DataSource {
    std::string fog_node;
    int num_samples;
    
public:
    DataSource(const std::string& fog, int samples)
        : fog_node(fog), num_samples(samples) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("Data source '%s' started", this_host->get_cname());
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.5, 2.0);
        
        for (int i = 0; i < num_samples; i++) {
            // Generate data (simulated)
            double wait_time = dis(gen);
            sg4::this_actor::sleep_for(wait_time);
            
            // Send to Fog node
            auto* mbox = sg4::Mailbox::by_name(fog_node);
            auto* data = new double(100.0 * dis(gen));  // Sensor value
            
            XBT_INFO("Sending sample #%d to Fog node", i + 1);
            mbox->put(data, 1e4);  // 10 KB
        }
        
        // Completion signal
        auto* mbox = sg4::Mailbox::by_name(fog_node);
        mbox->put(new std::string("FIN"), 100);
        
        XBT_INFO("All samples sent");
    }
};

// Actor for Fog node (processor and aggregator)
class FogAnalyzer {
    int num_sources;
    
public:
    FogAnalyzer(int sources) : num_sources(sources) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("Fog node '%s' started for analysis", this_host->get_cname());
        
        auto* mbox = sg4::Mailbox::by_name(this_host->get_cname());
        
        std::vector<double> all_data;
        int finished_sources = 0;
        
        while (finished_sources < num_sources) {
            auto* msg = mbox->get<void>();
            
            // Check if it's completion signal
            auto* fin_msg = dynamic_cast<std::string*>(static_cast<std::string*>(msg));
            if (fin_msg && *fin_msg == "FIN") {
                finished_sources++;
                delete fin_msg;
                XBT_INFO("Source completed (%d/%d)", finished_sources, num_sources);
                continue;
            }
            
            // Process data
            auto* data = static_cast<double*>(msg);
            all_data.push_back(*data);
            delete data;
            
            // Real-time analysis
            sg4::this_actor::execute(1e8);  // 100 MFlops per sample
        }
        
        // Final analysis and aggregation
        XBT_INFO("Performing aggregate analysis of %zu samples...", all_data.size());
        sg4::this_actor::execute(all_data.size() * 5e7);  // Complete analysis
        
        if (!all_data.empty()) {
            double sum = 0;
            for (double val : all_data) {
                sum += val;
            }
            double avg = sum / all_data.size();
            
            XBT_INFO("=== Analysis Results ===");
            XBT_INFO("  Samples processed: %zu", all_data.size());
            XBT_INFO("  Average: %.2f", avg);
            XBT_INFO("  Total sum: %.2f", sum);
        }
        
        XBT_INFO("Fog analysis completed");
    }
};

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform_file.xml>", argv[0]);
        XBT_CRITICAL("Example: %s platforms/fog_platform.xml", argv[0]);
        return 1;
    }
    
    e.load_platform(argv[1]);
    
    std::vector<sg4::Host*> hosts = e.get_all_hosts();
    
    if (hosts.size() < 2) {
        XBT_CRITICAL("At least 2 hosts required (1 Fog + 1 Edge)");
        return 1;
    }
    
    XBT_INFO("=== Fog Analytics Application ===");
    XBT_INFO("Platform loaded with %zu hosts", hosts.size());
    
    // Split hosts: first ones as Fog, rest as Edge
    size_t num_fog = std::max(1UL, hosts.size() / 3);
    std::vector<sg4::Host*> fog_nodes(hosts.begin(), hosts.begin() + num_fog);
    std::vector<sg4::Host*> edge_devices(hosts.begin() + num_fog, hosts.end());
    
    XBT_INFO("Fog nodes: %zu", fog_nodes.size());
    XBT_INFO("Edge devices: %zu", edge_devices.size());
    
    // Calculate devices per Fog node
    int devices_per_fog = edge_devices.size() / fog_nodes.size();
    if (devices_per_fog == 0) devices_per_fog = 1;
    
    // Create Fog actors
    for (auto* fog : fog_nodes) {
        fog->add_actor("fog_analyzer", FogAnalyzer(devices_per_fog));
        XBT_INFO("Fog node: %s", fog->get_cname());
    }
    
    // Create Edge actors
    int fog_idx = 0;
    for (size_t i = 0; i < edge_devices.size(); i++) {
        std::string fog_name = fog_nodes[fog_idx]->get_cname();
        int samples = 5 + (i % 5);  // 5-10 samples
        
        edge_devices[i]->add_actor("data_source_" + std::to_string(i),
                                    DataSource(fog_name, samples));
        
        fog_idx = (fog_idx + 1) % fog_nodes.size();
    }
    
    // Run simulation
    e.run();
    
    XBT_INFO("=== Simulation completed ===");
    XBT_INFO("Simulated time: %.2f seconds", sg4::Engine::get_clock());
    
    return 0;
}
