#include <simgrid/s4u.hpp>
#include <iostream>
#include <string>

XBT_LOG_NEW_DEFAULT_CATEGORY(edge_computing, "Edge Computing Application");

namespace sg4 = simgrid::s4u;

/**
 * @brief Edge processing application
 * 
 * Simulates Edge devices that process data locally and send
 * results to a gateway when necessary.
 */

// Actor for Edge device
class EdgeDevice {
    std::string gateway_name;
    double computation_size;
    double data_size;
    
public:
    EdgeDevice(const std::string& gateway, double comp_size, double data_size_)
        : gateway_name(gateway), computation_size(comp_size), data_size(data_size_) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("Edge Device '%s' started (speed: %.2f Gf)", 
                 this_host->get_cname(), this_host->get_speed() / 1e9);
        
        // Phase 1: Data collection
        XBT_INFO("Collecting sensor data...");
        sg4::this_actor::sleep_for(1.0);
        
        // Phase 2: Local processing
        XBT_INFO("Processing data locally (%.2f MFlops)...", computation_size / 1e6);
        sg4::this_actor::execute(computation_size);
        
        // Phase 3: Send results to gateway
        XBT_INFO("Sending results to gateway (%s)...", gateway_name.c_str());
        
        auto* mbox = sg4::Mailbox::by_name(gateway_name);
        auto* payload = new std::string("Processed data from " + 
                                        std::string(this_host->get_cname()));
        mbox->put(payload, data_size);
        
        XBT_INFO("Task completed");
    }
};

// Actor for Edge Gateway
class EdgeGateway {
    int num_devices;
    
public:
    EdgeGateway(int num) : num_devices(num) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("Edge Gateway '%s' started", this_host->get_cname());
        
        auto* mbox = sg4::Mailbox::by_name(this_host->get_cname());
        
        // Receive data from all devices
        for (int i = 0; i < num_devices; i++) {
            auto* msg = mbox->get<std::string>();
            XBT_INFO("Received: %s", msg->c_str());
            delete msg;
            
            // Process aggregated data
            sg4::this_actor::execute(5e8);  // 500 MFlops
        }
        
        XBT_INFO("All data processed. Gateway completed.");
    }
};

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    
    // Verify arguments
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform_file.xml>", argv[0]);
        XBT_CRITICAL("Example: %s platforms/edge_platform.xml", argv[0]);
        return 1;
    }
    
    // Load platform
    e.load_platform(argv[1]);
    
    // Get hosts
    std::vector<sg4::Host*> hosts = e.get_all_hosts();
    
    if (hosts.empty()) {
        XBT_CRITICAL("No hosts found in platform");
        return 1;
    }
    
    XBT_INFO("=== Edge Computing Application ===");
    XBT_INFO("Platform loaded with %zu hosts", hosts.size());
    
    // Find gateway (assume it's the first or has "gateway" in the name)
    sg4::Host* gateway = nullptr;
    std::vector<sg4::Host*> devices;
    
    for (auto* host : hosts) {
        std::string name = host->get_cname();
        if (name.find("gateway") != std::string::npos) {
            gateway = host;
        } else {
            devices.push_back(host);
        }
    }
    
    // If no explicit gateway, use first host
    if (!gateway && !hosts.empty()) {
        gateway = hosts[0];
        devices.clear();
        for (size_t i = 1; i < hosts.size(); i++) {
            devices.push_back(hosts[i]);
        }
    }
    
    if (!gateway || devices.empty()) {
        XBT_CRITICAL("At least one gateway and one device are required");
        return 1;
    }
    
    XBT_INFO("Gateway: %s", gateway->get_cname());
    XBT_INFO("Edge Devices: %zu", devices.size());
    
    // Create gateway actor
    gateway->add_actor("gateway", EdgeGateway(devices.size()));
    
    // Create actors for edge devices
    for (size_t i = 0; i < devices.size(); i++) {
        double comp_size = 1e9;  // 1 GFlop
        double data_size = 1e6;  // 1 MB
        
        devices[i]->add_actor("device_" + std::to_string(i),
                              EdgeDevice(gateway->get_cname(), comp_size, data_size));
    }
    
    // Run simulation
    e.run();
    
    XBT_INFO("=== Simulation completed ===");
    XBT_INFO("Simulated time: %.2f seconds", sg4::Engine::get_clock());
    
    return 0;
}
