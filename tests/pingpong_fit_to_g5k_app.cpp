#include <simgrid/s4u.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>

XBT_LOG_NEW_DEFAULT_CATEGORY(ping_pong_latency, "Ping-Pong Latency Measurement");

namespace sg4 = simgrid::s4u;

// Message structure for ping-pong
struct PingPongMessage {
    int message_id;
    size_t size_bytes;
    double send_time;
    bool is_pong;  // false = ping, true = pong
    std::vector<char> payload;
    
    PingPongMessage(int id, size_t size, double time, bool pong = false) 
        : message_id(id), size_bytes(size), send_time(time), is_pong(pong) {
        payload.resize(size, 'X');  // Fill with dummy data to reach the desired size
    }
};

// (No per-message stats; we use aggregate timing outside the loop)

/**
 * @brief Edge device actor - Sends ping messages and receives pong responses
 */
class EdgeDevice {
public:
    explicit EdgeDevice(bool quiet=false) : quiet_(quiet) {}
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        if(!quiet_) XBT_INFO("[EDGE] Device '%s' started", this_host->get_cname());
        
        // Define message sizes: 128B to 1MiB
        std::vector<size_t> message_sizes = {
            128,           // 128 B
            256,           // 256 B
            512,           // 512 B
            1024,          // 1 KiB
            2048,          // 2 KiB
            4096,          // 4 KiB
            8192,          // 8 KiB
            16384,         // 16 KiB
            32768,         // 32 KiB
            65536,         // 64 KiB
            131072,        // 128 KiB
            262144,        // 256 KiB
            524288,        // 512 KiB
            1048576        // 1 MiB
        };
        
        const int MESSAGES_PER_SIZE = 1000;
        
        // Get mailboxes for communication
        sg4::Mailbox* fog_mbox = sg4::Mailbox::by_name("fog_mailbox");
        sg4::Mailbox* edge_mbox = sg4::Mailbox::by_name("edge_mailbox");
        
        if(!quiet_) {
            XBT_INFO("[EDGE] Starting ping-pong latency test");
            XBT_INFO("[EDGE] Testing %zu message sizes with %d messages each", 
                     message_sizes.size(), MESSAGES_PER_SIZE);
            XBT_INFO("===========================================");
        }
        // For quiet summary rows
        struct SummaryRow { size_t size_bytes; int messages; double avg_ms; double thr_MBps; };
        std::vector<SummaryRow> rows;
        rows.reserve(message_sizes.size());

        // For each message size
        for (size_t msg_size : message_sizes) {
            if(!quiet_) XBT_INFO("[EDGE] Testing message size: %zu bytes", msg_size);
            double start_time = sg4::Engine::get_clock();
            for (int i = 0; i < MESSAGES_PER_SIZE; i++) {
                auto* ping = new PingPongMessage(i, msg_size, 0.0, false);
                fog_mbox->put(ping, msg_size);
                auto* pong = edge_mbox->get<PingPongMessage>();
                delete pong;
            }

            double end_time = sg4::Engine::get_clock();
            double avg_s = (end_time - start_time) / static_cast<double>(MESSAGES_PER_SIZE);
            double avg_ms = avg_s * 1000.0;
            double thr_MBps = avg_s > 0.0 ? ((msg_size * 2) / 1024.0 / 1024.0) / avg_s : 0.0;
            rows.push_back(SummaryRow{msg_size, MESSAGES_PER_SIZE, avg_ms, thr_MBps});

            if(!quiet_) {
                XBT_INFO("[EDGE] === Statistics for %zu bytes ===", msg_size);
                XBT_INFO("[EDGE]   Messages sent: %d", MESSAGES_PER_SIZE);
                XBT_INFO("[EDGE]   Avg latency:    %.6f ms", avg_ms);
                XBT_INFO("[EDGE]   Throughput:     %.4f MBps", thr_MBps);
                XBT_INFO("===========================================");
            }
        }
        // Quiet summary
        if(quiet_) {
            XBT_INFO("MessageSize\tMessages\tAvgLatency_ms\tThroughput_MBps\n");
            for(const auto& r : rows) {
                XBT_INFO("%zu\t\t%d\t\t%.3f\t\t%.4f\n", r.size_bytes, r.messages, r.avg_ms, r.thr_MBps);
            }
        }
        
        // Send termination signal
        sg4::Mailbox* fog_done_mbox = sg4::Mailbox::by_name("fog_done_mailbox");
        fog_done_mbox->put(new std::string("DONE"), 4);
        
        if(!quiet_) XBT_INFO("[EDGE] Device '%s' finished", this_host->get_cname());
    }
private:
    bool quiet_ = false;
};

/**
 * @brief Fog node actor - Receives ping messages and sends pong responses
 */
class FogNode {
public:
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
    XBT_INFO("[FOG] Node '%s' started", this_host->get_cname());
        
        sg4::Mailbox* fog_mbox = sg4::Mailbox::by_name("fog_mailbox");
        sg4::Mailbox* fog_done_mbox = sg4::Mailbox::by_name("fog_done_mailbox");
        sg4::Mailbox* edge_mbox = sg4::Mailbox::by_name("edge_mailbox");
        
        XBT_INFO("[FOG] Ready to receive ping messages");
        
        int messages_received = 0;
        bool running = true;
        
        // Keep receiving messages until termination signal
        while (running) {
            // Check for termination signal (non-blocking)
            if (fog_done_mbox->listen()) {
                auto* done_msg = fog_done_mbox->get<std::string>();
                XBT_INFO("[FOG] Received termination signal");
                delete done_msg;
                running = false;
                break;
            }
            
            // Check for ping messages
            if (fog_mbox->listen()) {
                auto* ping = fog_mbox->get<PingPongMessage>();
                messages_received++;
                
                // Create pong response with the same size
                auto* pong = new PingPongMessage(
                    ping->message_id, 
                    ping->size_bytes, 
                    sg4::Engine::get_clock(), 
                    true  // is_pong = true
                );
                // Send pong back
                edge_mbox->put(pong, ping->size_bytes);
                
                delete ping;
            }
            
            // Small yield to avoid busy waiting
            if (!fog_mbox->listen() && !fog_done_mbox->listen()) {
                sg4::this_actor::yield();
            }
        }
        
        XBT_INFO("[FOG] Node '%s' finished - Processed %d messages", 
                 this_host->get_cname(), messages_received);
        XBT_INFO("\n");
    }
};

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    bool quiet=false;
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform_file.xml> [--quiet]", argv[0]);
        return 1;
    }
    // Parse flags
    for(int i=2;i<argc;i++){ std::string a=argv[i]; if(a=="--quiet"||a=="-q") quiet=true; }
    e.load_platform(argv[1]);
    
    std::vector<sg4::Host*> hosts = e.get_all_hosts();
    if(!quiet) XBT_INFO("===========================================");
    if(!quiet) {
        XBT_INFO("=== PING-PONG LATENCY MEASUREMENT TEST ===");
        XBT_INFO("===========================================");
    }
    XBT_INFO("Platform loaded with %zu hosts", hosts.size());
    XBT_INFO("Configuration:");
    XBT_INFO("  Edge clusters: 1 × 1 nodes");
    XBT_INFO("  Fog clusters: 1 × 1 nodes");
    XBT_INFO("  Cloud clusters: 0 × 0 nodes");
    
    // Classify hosts by cluster type
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
    
    if(!quiet) {
        XBT_INFO("Detected: %zu edge, %zu fog, %zu cloud hosts", 
                 edge_hosts.size(), fog_hosts.size(), cloud_hosts.size());
        XBT_INFO("Message sizes: 128B to 1MiB");
        XBT_INFO("Messages per size: 1000");
        XBT_INFO("===========================================");
    }
    
    // Deploy Edge devices
    for (auto* host : edge_hosts) {
        host->add_actor("edge_device", EdgeDevice(quiet));
    }
    
    // Deploy Fog nodes
    for (auto* host : fog_hosts) {
        host->add_actor("fog_node", FogNode());
    }
    
    // Run simulation
    e.run();
    

    XBT_INFO("===========================================");
    XBT_INFO("=== Simulation completed ===");
    XBT_INFO("Simulated time: %.2f seconds", sg4::Engine::get_clock());
    XBT_INFO("===========================================");
    
    
    return 0;
}
