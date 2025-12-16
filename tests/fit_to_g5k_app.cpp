#include <simgrid/s4u.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>

XBT_LOG_NEW_DEFAULT_CATEGORY(distributed_write, "Distributed Block Write to Fog Servers");

namespace sg4 = simgrid::s4u;

// Configuration constants
const size_t BLOCK_SIZE = 512 * 1024;  // 512 KiB per block
bool QUIET_MODE = false;  // Global flag for quiet mode
int RNG_SEED = 42;        // Global seed for reproducible randomization
enum DistributionMode { RANDOM, ROUND_ROBIN };
DistributionMode DIST_MODE = RANDOM;  // Distribution mode for initial fog selection
bool ENABLE_ACK = true;   // Whether IoTs wait for ACKs from fog servers
int NUM_FOG = 0;

// Message types
enum MessageType {
    WRITE_REQUEST,
    WRITE_ACK,
    TERMINATION
};

// Write request message structure
// Note: SimGrid simulates data transmission time based on the size parameter
// in put()/get() calls, so we don't need to actually allocate payload data.
struct WriteMessage {
    MessageType type;
    int edge_id;
    int block_id;
    int fog_server_id;
    size_t data_size;
    double send_time;
    
    WriteMessage(MessageType t, int eid, int bid, int fid, size_t size, double time) 
        : type(t), edge_id(eid), block_id(bid), fog_server_id(fid), 
          data_size(size), send_time(time) {
        // No need to allocate payload - SimGrid uses the size parameter in put() for simulation
    }
};

// Statistics per edge device
struct EdgeStats {
    int total_messages;
    int total_blocks;
    size_t total_bytes;
    double total_time;
    int total_acks_received;
    std::map<int, int> messages_per_fog;  // fog_id -> message_count
    std::map<int, int> blocks_per_fog;    // fog_id -> block_count
    
    EdgeStats() : total_messages(0), total_blocks(0), total_bytes(0), total_time(0.0), total_acks_received(0) {}
    
    void print_stats(int edge_id) {
        XBT_INFO("[EDGE %d] === Final Statistics ===", edge_id);
        XBT_INFO("[EDGE %d]   Total blocks sent: %d", edge_id, total_blocks);
        XBT_INFO("[EDGE %d]   Total messages sent: %d", edge_id, total_messages);
        XBT_INFO("[EDGE %d]   Total data written: %.2f MiB", edge_id, total_bytes / (1024.0 * 1024.0));

    }
};

/**
 * @brief Edge device actor - Writes messages distributed across fog servers
 */
class EdgeDevice {
private:
    int edge_id_;
    int num_messages_;
    int num_fog_servers_;
    size_t message_size_;

public:
    EdgeDevice(int edge_id, int num_messages, int num_fog_servers, size_t message_size) 
        : edge_id_(edge_id), num_messages_(num_messages), num_fog_servers_(num_fog_servers), message_size_(message_size) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        if (!QUIET_MODE) {
            XBT_INFO("[EDGE %d] Device '%s' started - Writing %d messages to %d fog servers", 
                     edge_id_, this_host->get_cname(), num_messages_, num_fog_servers_);
        }
        
        EdgeStats stats;
        
        // Message size - configurable per run
        const size_t MESSAGE_SIZE = message_size_;  // bytes per message
        
        // Choose initial fog based on distribution mode
        int current_fog_id;
        if (DIST_MODE == RANDOM) {
            // Random distribution: use seeded random generator (uniform distribution)
            std::mt19937 rng(static_cast<uint32_t>(RNG_SEED + edge_id_));
            std::uniform_int_distribution<int> fog_dist(0, num_fog_servers_ - 1);
            current_fog_id = fog_dist(rng);
        } else {
            //current_fog_id = 0;
            current_fog_id = edge_id_ % num_fog_servers_;
        }
        
        double start_time = sg4::Engine::get_clock();
        
        // Create mailbox for receiving ACKs
        std::string edge_mbox_name = "edge_" + std::to_string(edge_id_);
        sg4::Mailbox* edge_mbox = sg4::Mailbox::by_name(edge_mbox_name);
        
        if (!QUIET_MODE) {
            XBT_INFO("[EDGE %d] Starting write operations (msg size: %zu bytes, starting fog: %d, mode: %s)", 
                     edge_id_, MESSAGE_SIZE, current_fog_id, 
                     DIST_MODE == RANDOM ? "random" : "round-robin");
        }
        
        // Block-based writing logic
        size_t current_block_size = 0;
        int current_block_id = 0;
        
        for (int msg_id = 0; msg_id < num_messages_; ++msg_id) {
            // Check if we need to start a new block
            if (current_block_size + MESSAGE_SIZE > BLOCK_SIZE) {
                // Block is full, move to next fog server and start new block
                current_fog_id = (current_fog_id + 1) % num_fog_servers_;
                current_block_size = 0;
                current_block_id++;
                stats.blocks_per_fog[current_fog_id]++;
            }
            
            // If this is the first message of a block
            if (current_block_size == 0) {
                stats.blocks_per_fog[current_fog_id]++;
            }
            
            // Send write request to current fog server
            std::string fog_mbox_name = "fog_" + std::to_string(current_fog_id);
            sg4::Mailbox* fog_mbox = sg4::Mailbox::by_name(fog_mbox_name);
            
            auto* write_msg = new WriteMessage(
                WRITE_REQUEST,
                edge_id_,
                current_block_id,
                current_fog_id,
                MESSAGE_SIZE,
                sg4::Engine::get_clock()
            );
            
            if (ENABLE_ACK) {
                // Send write request asynchronously and wait for ACK
                fog_mbox->put(write_msg, MESSAGE_SIZE);
                
                // Wait for ACK from fog server
                auto* ack_msg = edge_mbox->get<WriteMessage>();
                if (ack_msg->type == WRITE_ACK) {
                    stats.total_acks_received++;
                    if (!QUIET_MODE && (msg_id + 1) % 1000 == 0) {
                        XBT_INFO("[EDGE %d] Received ACK %d from Fog %d", 
                                 edge_id_, stats.total_acks_received, ack_msg->fog_server_id);
                    }
                }
                delete ack_msg;
            } else {
                // Fire-and-forget: no ACK expected
                fog_mbox->put(write_msg, MESSAGE_SIZE);
            }
            
            stats.total_messages++;
            stats.total_bytes += MESSAGE_SIZE;
            stats.messages_per_fog[current_fog_id]++;
            current_block_size += MESSAGE_SIZE;
            
            sg4::this_actor::sleep_for(0.1);
        }
        
        stats.total_blocks = current_block_id + 1;
        
        
        double end_time = sg4::Engine::get_clock();
        stats.total_time = end_time - start_time;
        
        // Send termination signals to all fog servers
        for (int fid = 0; fid < num_fog_servers_; ++fid) {
            std::string fog_mbox_name = "fog_" + std::to_string(fid);
            sg4::Mailbox* fog_mbox = sg4::Mailbox::by_name(fog_mbox_name);
            auto* term_msg = new WriteMessage(TERMINATION, edge_id_, -1, fid, 0, 0.0);
            fog_mbox->put(term_msg, 0);
        }
        
        stats.print_stats(edge_id_);
        if (!QUIET_MODE) {
            XBT_INFO("[EDGE %d] Device '%s' finished", edge_id_, this_host->get_cname());
        }
        XBT_INFO("\n");
    }
};

// Statistics per fog server (including IOPS)
struct FogStats {
    int fog_id;
    int total_operations;
    double start_time;
    double end_time;
    double ops_per_second;
    
    FogStats() : fog_id(-1), total_operations(0), start_time(0.0), end_time(0.0), ops_per_second(0.0) {}
    FogStats(int id) : fog_id(id), total_operations(0), start_time(0.0), end_time(0.0), ops_per_second(0.0) {}
};

// Global storage for fog statistics (using MutexPtr instead of direct Mutex)
std::map<int, FogStats> global_fog_stats;

/**
 * @brief Fog server actor - Receives write requests from edge devices
 */
class FogServer {
private:
    int fog_id_;
    int num_edge_devices_;
    
public:
    FogServer(int fog_id, int num_edge_devices) 
        : fog_id_(fog_id), num_edge_devices_(num_edge_devices) {}
    
    void operator()() {
        sg4::Host* this_host = sg4::this_actor::get_host();
        if (!QUIET_MODE) {
            XBT_INFO("[FOG %d] Server '%s' started - Waiting for writes from %d edge devices", 
                     fog_id_, this_host->get_cname(), num_edge_devices_);
        }
        
        std::string fog_mbox_name = "fog_" + std::to_string(fog_id_);
        sg4::Mailbox* fog_mbox = sg4::Mailbox::by_name(fog_mbox_name);
        
        int total_writes = 0;
        size_t total_bytes = 0;
        std::map<int, int> writes_per_edge;
        int terminations_received = 0;
        
        double first_operation_time = -1.0;
        double last_operation_time = -1.0;
        
        while (terminations_received < num_edge_devices_) {
            // Non-blocking check for messages
            auto* msg = fog_mbox->get<WriteMessage>();
            
            if (msg->type == TERMINATION) {
                terminations_received++;
                if (!QUIET_MODE) {
                    XBT_INFO("[FOG %d] Received termination from Edge %d (%d/%d)", 
                             fog_id_, msg->edge_id, terminations_received, num_edge_devices_);
                }
                delete msg;
                continue;
            }
            
            // Record time of first operation
            if (first_operation_time < 0.0) {
                first_operation_time = sg4::Engine::get_clock();
            }
            
            // Process write request and send ACK back to edge device (if enabled)
            // Each write = 1 operation (open-write-close)
            total_writes++;
            total_bytes += msg->data_size;
            writes_per_edge[msg->edge_id]++;
            // Record time of last write operation received
            last_operation_time = sg4::Engine::get_clock();

            // Send ACK back to the edge device asynchronously (only if ACKs are enabled)
            if (ENABLE_ACK) {
                std::string edge_mbox_name = "edge_" + std::to_string(msg->edge_id);
                sg4::Mailbox* edge_mbox = sg4::Mailbox::by_name(edge_mbox_name);
                
                auto* ack_msg = new WriteMessage(
                    WRITE_ACK,
                    msg->edge_id,
                    msg->block_id,
                    fog_id_,
                    0,  // ACK has no payload
                    sg4::Engine::get_clock()
                );
                
                edge_mbox->put(ack_msg, 0);  // ACK sent asynchronously
            }
            
            delete msg;
        }
        
        double end_time = last_operation_time;
        double active_time = (first_operation_time > 0.0) ? (end_time - first_operation_time) : 0.0;
        double ops_per_sec = (active_time > 0.0) ? (total_writes / active_time) : 0.0;
        
        // Store stats globally for aggregation (no mutex needed, SimGrid actors don't run in parallel)
        FogStats stats(fog_id_);
        stats.total_operations = total_writes;
        stats.start_time = first_operation_time;
        stats.end_time = end_time;
        stats.ops_per_second = ops_per_sec;
        global_fog_stats[fog_id_] = stats;
        
        XBT_INFO("[FOG %d] === Final Statistics ===", fog_id_);
        XBT_INFO("[FOG %d]   Total writes received: %d", fog_id_, total_writes);
        XBT_INFO("[FOG %d]   Total data received: %.2f MiB", fog_id_, total_bytes / (1024.0 * 1024.0));
        XBT_INFO("[FOG %d]   First operation received at: %.4f seconds", fog_id_, first_operation_time);
        XBT_INFO("[FOG %d]   Last operation received at: %.4f seconds", fog_id_, last_operation_time);
        XBT_INFO("[FOG %d]   Active time (last - first): %.4f seconds", fog_id_, active_time);
        XBT_INFO("[FOG %d]   Operations per second (IOPS): %.2f ops/s", fog_id_, ops_per_sec);
        XBT_INFO("[FOG %d]   Writes per edge device:", fog_id_);
        /*for (const auto& entry : writes_per_edge) {
            XBT_INFO("[FOG %d]     Edge %d: %d writes", fog_id_, entry.first, entry.second);
        }*/
        
        if (!QUIET_MODE) {
            XBT_INFO("[FOG %d] Server '%s' finished", fog_id_, this_host->get_cname());
        }
        XBT_INFO("\n");
    }
};

int main(int argc, char* argv[]) {
    sg4::Engine e(&argc, argv);
    
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform_file.xml> [num_messages=1000] [num_fog_servers=4] [packet_size_bytes=4096] [options]", 
                     argv[0]);
        XBT_CRITICAL("  num_messages: Number of messages each edge device will write (default: 1000)");
        XBT_CRITICAL("  num_fog_servers: Number of fog servers to distribute writes across (default: 4, can be 4, 8, or 16)");
        XBT_CRITICAL("  packet_size_bytes: Size of each message packet in bytes (default: 4096)");
        XBT_CRITICAL("Options:");
        XBT_CRITICAL("  --quiet or -q: Quiet mode - show only final statistics");
        XBT_CRITICAL("  --seed <int>: Set RNG seed for reproducibility (default: 42)");
        XBT_CRITICAL("  --distribution or -d <mode>: Initial fog distribution mode:");
        XBT_CRITICAL("    random|rand: Random uniform distribution (default)");
        XBT_CRITICAL("    rr|round-robin: Round-robin distribution based on edge ID");
        XBT_CRITICAL("  --ack: Enable ACK mode - IoTs wait for acknowledgments from fog servers (default)");
        XBT_CRITICAL("  --no-ack: Disable ACK mode - Fire-and-forget communication");
        return 1;
    }
    
    e.load_platform(argv[1]);
    
    // Parse command line arguments
    int num_messages = (argc > 2) ? std::atoi(argv[2]) : 1000;
    int num_fog_servers = (argc > 3) ? std::atoi(argv[3]) : 4;
    int packet_size = (argc > 4) ? std::atoi(argv[4]) : 4096; // bytes per message

    NUM_FOG = num_fog_servers;
    
    // Parse optional flags: quiet mode, RNG seed, distribution mode, and ACK mode
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--quiet" || arg == "-q") {
            QUIET_MODE = true;
        } else if (arg == "--seed" && i + 1 < argc) {
            RNG_SEED = std::atoi(argv[i + 1]);
            i++;
        } else if ((arg == "--distribution" || arg == "-d") && i + 1 < argc) {
            std::string mode = argv[i + 1];
            if (mode == "random" || mode == "rand") {
                DIST_MODE = RANDOM;
            } else if (mode == "rr" || mode == "round-robin") {
                DIST_MODE = ROUND_ROBIN;
            } else {
                XBT_WARN("Unknown distribution mode '%s', using 'random'", mode.c_str());
                DIST_MODE = RANDOM;
            }
            i++;
        } else if (arg == "--ack") {
            ENABLE_ACK = true;
        } else if (arg == "--no-ack") {
            ENABLE_ACK = false;
        }
    }
    
    // Validate parameters
    if (num_messages <= 0) {
        XBT_WARN("num_messages must be > 0, using default: 1000");
        num_messages = 1000;
    }
    
    if (num_fog_servers != 4 && num_fog_servers != 8 && num_fog_servers != 16) {
        XBT_WARN("num_fog_servers should be 4, 8, or 16, using default: 4");
        num_fog_servers = 4;
    }

    if (packet_size <= 0 || static_cast<size_t>(packet_size) > BLOCK_SIZE) {
        XBT_WARN("packet_size must be >0 and <= %zu, using default 4096", BLOCK_SIZE);
        packet_size = 4096;
    }
    
    std::vector<sg4::Host*> hosts = e.get_all_hosts();

    XBT_INFO("=======================================================");
    XBT_INFO("=== START OF THE SIMULATION ===");
    XBT_INFO("=======================================================");
    XBT_INFO("Platform loaded with %zu hosts", hosts.size());
    XBT_INFO("Configuration:");
    XBT_INFO("  Messages per edge device: %d", num_messages);
    XBT_INFO("  Number of fog servers: %d", num_fog_servers);
    XBT_INFO("  Block size: %zu KiB", BLOCK_SIZE / 1024);
    XBT_INFO("  Message size: %d bytes", packet_size);
    XBT_INFO("  RNG seed: %d", RNG_SEED);
    XBT_INFO("  Distribution mode: %s", DIST_MODE == RANDOM ? "random" : "round-robin");
    XBT_INFO("  ACK mode: %s", ENABLE_ACK ? "enabled" : "disabled (fire-and-forget)");
    XBT_INFO("\n");
    
    // Classify hosts by cluster type
    std::vector<sg4::Host*> edge_hosts, fog_hosts;
    
    for (auto* host : hosts) {
        std::string name = host->get_cname();
        if (name.find("edge") != std::string::npos) {
            edge_hosts.push_back(host);
        } else if (name.find("fog") != std::string::npos) {
            fog_hosts.push_back(host);
        }
    }
    
    if (!QUIET_MODE) {
        XBT_INFO("Detected: %zu edge devices, %zu fog servers", 
                 edge_hosts.size(), fog_hosts.size());
    }
    
    // Verify we have enough fog servers
    if (fog_hosts.size() < static_cast<size_t>(num_fog_servers)) {
        XBT_CRITICAL("Platform only has %zu fog servers, but %d requested", 
                     fog_hosts.size(), num_fog_servers);
        XBT_CRITICAL("Please generate a platform with at least %d fog servers", num_fog_servers);
        return 1;
    }
    
    if (!QUIET_MODE) {
        XBT_INFO("=======================================================");
    }
    
    // Deploy Fog servers (only the requested number)
    for (int i = 0; i < num_fog_servers; i++) {
        fog_hosts[i]->add_actor("fog_server_" + std::to_string(i),
                                FogServer(i, edge_hosts.size()));
    }
    
    // Deploy Edge devices
    for (size_t i = 0; i < edge_hosts.size(); i++) {
        edge_hosts[i]->add_actor("edge_device_" + std::to_string(i),
                                 EdgeDevice(static_cast<int>(i), num_messages, num_fog_servers, static_cast<size_t>(packet_size)));
    }
    
    // Run simulation
    e.run();
    
    // Compute global IOPS statistics using worst (max) fog active time
    int total_operations = 0;
    double worst_active_time = 0.0; // max over fogs of (end_time - start_time)

    for (const auto& entry : global_fog_stats) {
        const FogStats& stats = entry.second;
        total_operations += stats.total_operations;
        if (stats.start_time > 0.0 && stats.end_time > stats.start_time) {
            double fog_active_time = stats.end_time - stats.start_time;
            if (fog_active_time > worst_active_time)
                worst_active_time = fog_active_time;
        }
    }

    double global_ops_per_sec = (worst_active_time > 0.0) ? (total_operations / worst_active_time) : 0.0;

    XBT_INFO("=======================================================");
    XBT_INFO("=== Simulation completed ===");
    XBT_INFO("Simulated time: %.2f seconds", sg4::Engine::get_clock());
    XBT_INFO("=======================================================");
    XBT_INFO("=== GLOBAL FOG STATISTICS ===");
    XBT_INFO("Total operations: %d", total_operations);
    XBT_INFO("Worst fog active time: %.2f seconds", worst_active_time);
    XBT_INFO("Global IOPS (total ops / worst time): %.2f ops/s", global_ops_per_sec);
    XBT_INFO("=======================================================");
    
    return 0;
}
