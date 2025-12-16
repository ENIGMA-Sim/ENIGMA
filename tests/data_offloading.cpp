#include <simgrid/s4u.hpp>
#include <iostream>
#include <vector>

XBT_LOG_NEW_DEFAULT_CATEGORY(data_offloading, "Data Offloading Application");

namespace sg4 = simgrid::s4u;

/**
 * @brief Data offloading application
 * 
 * Simulates offloading decisions: process locally on Edge,
 * send to Fog, or send to Cloud based on load and resources.
 */

class SmartEdgeDevice {
    std::string fog_name;
    std::string cloud_name;
    double workload;
    int num_tasks;
    
public:
    SmartEdgeDevice(const std::string& fog, const std::string& cloud, double load, int tasks = 1)
        : fog_name(fog), cloud_name(cloud), workload(load), num_tasks(tasks) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        std::string result_mailbox_name = std::string(this_host->get_cname()) + "_result";
        
        XBT_INFO("Edge Device '%s' started (load: %.2f GFlops, tasks: %d)",
                 this_host->get_cname(), workload / 1e9, num_tasks);
        
        double device_capacity = this_host->get_speed();
        
        // Process multiple tasks
        for (int task_id = 0; task_id < num_tasks; task_id++) {
            XBT_INFO("--- Processing task %d/%d ---", task_id + 1, num_tasks);
            
            // Offloading decision based on capacity
            if (workload < device_capacity * 0.5) {
                // Local processing - low load
                XBT_INFO("Decision: LOCAL PROCESSING (low load)");
                double start_time = sg4::Engine::get_clock();
                sg4::this_actor::execute(workload);
                double end_time = sg4::Engine::get_clock();
                XBT_INFO("Local processing completed in %.2f seconds",
                         end_time - start_time);
                
            } else if (workload < device_capacity * 2.0) {
                // Offload to Fog - medium load
                XBT_INFO("Decision: OFFLOAD TO FOG (medium load)");
                
                auto* mbox = sg4::Mailbox::by_name(fog_name);
                
                // Create task payload with source info
                auto* task_data = new std::pair<double, std::string>(workload, result_mailbox_name);
                
                XBT_INFO("Sending task to Fog '%s' (%.2f MFlops)...",
                         fog_name.c_str(), workload / 1e6);
                mbox->put(task_data, workload / 1e3);  // Data size = workload/1000
                
                // Wait for result
                auto* result_mbox = sg4::Mailbox::by_name(result_mailbox_name);
                auto* result = result_mbox->get<std::string>();
                XBT_INFO("Result received from Fog: %s", result->c_str());
                delete result;
                
            } else {
                // Offload to Cloud - high load
                XBT_INFO("Decision: OFFLOAD TO CLOUD (high load)");
                
                auto* mbox = sg4::Mailbox::by_name(cloud_name);
                
                // Create task payload with source info
                auto* task_data = new std::pair<double, std::string>(workload, result_mailbox_name);
                
                XBT_INFO("Sending task to Cloud '%s' (%.2f MFlops)...",
                         cloud_name.c_str(), workload / 1e6);
                mbox->put(task_data, workload / 1e3);
                
                // Wait for result
                auto* result_mbox = sg4::Mailbox::by_name(result_mailbox_name);
                auto* result = result_mbox->get<std::string>();
                XBT_INFO("Result received from Cloud: %s", result->c_str());
                delete result;
            }
            
            // Small delay between tasks
            if (task_id < num_tasks - 1) {
                sg4::this_actor::sleep_for(0.5);
            }
        }
        
        XBT_INFO("All %d tasks completed successfully", num_tasks);
    }
};

class OffloadingServer {
    std::string type;
    int max_tasks;
    
public:
    OffloadingServer(const std::string& server_type, int max = -1) 
        : type(server_type), max_tasks(max) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        XBT_INFO("[%s] Server '%s' ready to receive tasks",
                 type.c_str(), this_host->get_cname());
        
        auto* mbox = sg4::Mailbox::by_name(this_host->get_cname());
        int tasks_processed = 0;
        
        // Server listening for tasks
        while (max_tasks < 0 || tasks_processed < max_tasks) {
            try {
                // Receive task with source mailbox info
                auto* task_data = mbox->get<std::pair<double, std::string>>(10.0);  // Timeout 10s
                
                double workload = task_data->first;
                std::string reply_to = task_data->second;
                
                XBT_INFO("[%s] Task received: %.2f MFlops (reply to: %s)", 
                         type.c_str(), workload / 1e6, reply_to.c_str());
                
                // Process task
                double start_time = sg4::Engine::get_clock();
                sg4::this_actor::execute(workload);
                double end_time = sg4::Engine::get_clock();
                
                XBT_INFO("[%s] Task processed in %.2f seconds",
                         type.c_str(), end_time - start_time);
                
                tasks_processed++;
                delete task_data;
                
                // Send result back to the requesting device
                auto* result_mbox = sg4::Mailbox::by_name(reply_to);
                auto* result = new std::string(
                    type + " processed task in " + 
                    std::to_string(end_time - start_time) + "s");
                
                result_mbox->put(result, 100);  // Small result message
                XBT_INFO("[%s] Result sent back to %s", type.c_str(), reply_to.c_str());
                
            } catch (const simgrid::TimeoutException&) {
                // Timeout - no more tasks
                XBT_INFO("[%s] No more tasks, finishing (processed %d tasks)", 
                         type.c_str(), tasks_processed);
                break;
            }
        }
        
        if (max_tasks > 0 && tasks_processed >= max_tasks) {
            XBT_INFO("[%s] Maximum tasks reached (%d), server finishing", 
                     type.c_str(), tasks_processed);
        }
        
        XBT_INFO("[%s] Server finished (total tasks: %d)", type.c_str(), tasks_processed);
    }
};

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform_file.xml> [num_tasks_per_device]", argv[0]);
        XBT_CRITICAL("Example: %s platforms/hybrid_platform.xml 3", argv[0]);
        XBT_CRITICAL("  - num_tasks_per_device: Number of tasks each edge device will send (default: 1)");
        return 1;
    }
    
    // Parse number of tasks per device (optional parameter)
    int num_tasks_per_device = 10;
    if (argc >= 3) {
        num_tasks_per_device = std::atoi(argv[2]);
        if (num_tasks_per_device < 1) {
            XBT_CRITICAL("num_tasks_per_device must be >= 1");
            return 1;
        }
    }
    
    e.load_platform(argv[1]);
    
    std::vector<sg4::Host*> hosts = e.get_all_hosts();
    
    if (hosts.size() < 3) {
        XBT_CRITICAL("At least 3 hosts required (Edge, Fog, Cloud)");
        return 1;
    }
    
    XBT_INFO("=== Data Offloading Application ===");
    XBT_INFO("Platform loaded with %zu hosts", hosts.size());
    XBT_INFO("Tasks per edge device: %d", num_tasks_per_device);
    
    // Classify hosts
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
    
    // Default distribution if no specific names
    if (edge_hosts.empty() && fog_hosts.empty() && cloud_hosts.empty()) {
        size_t third = hosts.size() / 3;
        edge_hosts.assign(hosts.begin(), hosts.begin() + third);
        fog_hosts.assign(hosts.begin() + third, hosts.begin() + 2 * third);
        cloud_hosts.assign(hosts.begin() + 2 * third, hosts.end());
    }
    
    if (edge_hosts.empty()) edge_hosts.push_back(hosts[0]);
    if (fog_hosts.empty()) fog_hosts.push_back(hosts[std::min(1UL, hosts.size()-1)]);
    if (cloud_hosts.empty()) cloud_hosts.push_back(hosts.back());
    
    XBT_INFO("Configuration:");
    XBT_INFO("  Edge devices: %zu", edge_hosts.size());
    XBT_INFO("  Fog nodes: %zu", fog_hosts.size());
    XBT_INFO("  Cloud servers: %zu", cloud_hosts.size());
    
    // Calculate expected total tasks for servers
    int expected_offloaded_tasks = 0;
    for (size_t i = 0; i < edge_hosts.size(); i++) {
        double workload = 1e9 * (0.5 + i * 0.5);
        double device_capacity = edge_hosts[i]->get_speed();
        // Only count tasks that will be offloaded
        if (workload >= device_capacity * 0.5) {
            expected_offloaded_tasks += num_tasks_per_device;
        }
    }
    
    XBT_INFO("  Expected offloaded tasks: %d", expected_offloaded_tasks);
    
    // Create Fog servers (with task limit based on expected load)
    for (auto* fog : fog_hosts) {
        fog->add_actor("fog_server", OffloadingServer("FOG", -1));  // -1 = unlimited
    }
    
    // Create Cloud servers (with task limit based on expected load)
    for (auto* cloud : cloud_hosts) {
        cloud->add_actor("cloud_server", OffloadingServer("CLOUD", -1));  // -1 = unlimited
    }
    
    // Create Edge devices with different loads
    double base_workload = 1e9;  // 1 GFlop base
    for (size_t i = 0; i < edge_hosts.size(); i++) {
        double workload = base_workload * (0.5 + i * 0.5);  // Increasing load
        
        std::string fog_name = fog_hosts[i % fog_hosts.size()]->get_cname();
        std::string cloud_name = cloud_hosts[i % cloud_hosts.size()]->get_cname();
        
        edge_hosts[i]->add_actor("smart_device_" + std::to_string(i),
                                 SmartEdgeDevice(fog_name, cloud_name, workload, num_tasks_per_device));
    }
    
    // Run simulation
    e.run();
    
    XBT_INFO("=== Simulation completed ===");
    XBT_INFO("Simulated time: %.2f seconds", sg4::Engine::get_clock());
    
    return 0;
}
