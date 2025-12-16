#include "platform/PlatformBuilder.hpp"
#include "platform/EdgePlatform.hpp"
#include "platform/FogPlatform.hpp"
#include "platform/CloudPlatform.hpp"
#include <iostream>
#include <string>
#include <fstream>

using namespace enigma;

void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " <type> [options]\n\n";
    std::cout << "Platform types:\n";
    std::cout << "  Simple (all hosts interconnected):\n";
    std::cout << "    edge <num_devices>            - Edge platform\n";
    std::cout << "    fog <num_nodes>               - Fog platform\n";
    std::cout << "    cloud <num_servers>           - Cloud platform\n";
    std::cout << "    iot <sensors> <actuators>     - IoT platform\n";
    std::cout << "\n  Cluster-based (organized in clusters):\n";
    std::cout << "    edge-cluster <num_clusters> <nodes_per_cluster>  - Edge clusters\n";
    std::cout << "    fog-cluster <num_clusters> <nodes_per_cluster>   - Fog clusters\n";
    std::cout << "    cloud-cluster <num_clusters> <nodes_per_cluster> - Cloud clusters\n";
    std::cout << "    hybrid-cluster <edge_clusters> <edge_nodes> <fog_clusters> <fog_nodes> <cloud_clusters> <cloud_nodes> [edge_cloud_direct] [output_file] [--generate-app] - Flat hybrid (optional direct Edge-Cloud + optional output filename + optional app template)\n";
    std::cout << "\nFlags:\n";
    std::cout << "    --generate-app    Generate a C++ template application for the platform\n";
    std::cout << "\nExamples:\n";
    std::cout << "  Simple:\n";
    std::cout << "    " << progName << " edge 10\n";
    std::cout << "\n  Clusters:\n";
    std::cout << "    " << progName << " edge-cluster 3 5       # 3 edge clusters with 5 devices each\n";
    std::cout << "    " << progName << " hybrid-cluster 2 10 2 5 1 20 1 custom_platform.xml --generate-app  # With template app\n";
}

void generateTemplateApp(const std::string& appFilename, const std::string& platformFile,
                         int edgeClusters, int edgeNodes, 
                         int fogClusters, int fogNodes,
                         int cloudClusters, int cloudNodes) {
    std::ofstream appFile(appFilename);
    if (!appFile.is_open()) {
        std::cerr << "Error: Could not create template app file: " << appFilename << std::endl;
        return;
    }
    
    appFile << "#include <simgrid/s4u.hpp>\n";
    appFile << "#include <iostream>\n";
    appFile << "#include <string>\n\n";
    appFile << "XBT_LOG_NEW_DEFAULT_CATEGORY(app_template, \"Application Template\");\n\n";
    appFile << "namespace sg4 = simgrid::s4u;\n\n";
    
    // Generate Edge actor class if there are edge clusters
    if (edgeClusters > 0) {
        appFile << "/**\n";
        appFile << " * @brief Edge device actor\n";
        appFile << " * TODO: Implement your edge device logic here\n";
        appFile << " */\n";
        appFile << "class EdgeDevice {\n";
        appFile << "public:\n";
        appFile << "    void operator()() {\n";
        appFile << "        sg4::Host* this_host = sg4::this_actor::get_host();\n";
        appFile << "        XBT_INFO(\"[EDGE] Device '%s' started\", this_host->get_cname());\n";
        appFile << "        \n";
        appFile << "        // TODO: Add your edge processing logic here\n";
        appFile << "        // Example: sense data, process locally, send to fog/cloud\n";
        appFile << "        \n";
        appFile << "        XBT_INFO(\"[EDGE] Device '%s' finished\", this_host->get_cname());\n";
        appFile << "    }\n";
        appFile << "};\n\n";
    }
    
    // Generate Fog actor class if there are fog clusters
    if (fogClusters > 0) {
        appFile << "/**\n";
        appFile << " * @brief Fog node actor\n";
        appFile << " * TODO: Implement your fog node logic here\n";
        appFile << " */\n";
        appFile << "class FogNode {\n";
        appFile << "public:\n";
        appFile << "    void operator()() {\n";
        appFile << "        sg4::Host* this_host = sg4::this_actor::get_host();\n";
        appFile << "        XBT_INFO(\"[FOG] Node '%s' started\", this_host->get_cname());\n";
        appFile << "        \n";
        appFile << "        // TODO: Add your fog processing logic here\n";
        appFile << "        // Example: receive from edge, aggregate, filter, forward to cloud\n";
        appFile << "        \n";
        appFile << "        XBT_INFO(\"[FOG] Node '%s' finished\", this_host->get_cname());\n";
        appFile << "    }\n";
        appFile << "};\n\n";
    }
    
    // Generate Cloud actor class if there are cloud clusters
    if (cloudClusters > 0) {
        appFile << "/**\n";
        appFile << " * @brief Cloud server actor\n";
        appFile << " * TODO: Implement your cloud server logic here\n";
        appFile << " */\n";
        appFile << "class CloudServer {\n";
        appFile << "public:\n";
        appFile << "    void operator()() {\n";
        appFile << "        sg4::Host* this_host = sg4::this_actor::get_host();\n";
        appFile << "        XBT_INFO(\"[CLOUD] Server '%s' started\", this_host->get_cname());\n";
        appFile << "        \n";
        appFile << "        // TODO: Add your cloud processing logic here\n";
        appFile << "        // Example: receive data, perform analytics, store results\n";
        appFile << "        \n";
        appFile << "        XBT_INFO(\"[CLOUD] Server '%s' finished\", this_host->get_cname());\n";
        appFile << "    }\n";
        appFile << "};\n\n";
    }
    
    // Generate main function
    appFile << "int main(int argc, char* argv[]) {\n";
    appFile << "    sg4::Engine e(&argc, argv);\n";
    appFile << "    \n";
    appFile << "    if (argc < 2) {\n";
    appFile << "        XBT_CRITICAL(\"Usage: %s <platform_file.xml>\", argv[0]);\n";
    appFile << "        return 1;\n";
    appFile << "    }\n";
    appFile << "    \n";
    appFile << "    e.load_platform(argv[1]);\n";
    appFile << "    \n";
    appFile << "    std::vector<sg4::Host*> hosts = e.get_all_hosts();\n";
    appFile << "    XBT_INFO(\"=== Application Template ===\");\n";
    appFile << "    XBT_INFO(\"Platform loaded with %zu hosts\", hosts.size());\n";
    appFile << "    XBT_INFO(\"Configuration:\");\n";
    appFile << "    XBT_INFO(\"  Edge clusters: " << edgeClusters << " × " << edgeNodes << " nodes\");\n";
    appFile << "    XBT_INFO(\"  Fog clusters: " << fogClusters << " × " << fogNodes << " nodes\");\n";
    appFile << "    XBT_INFO(\"  Cloud clusters: " << cloudClusters << " × " << cloudNodes << " nodes\");\n";
    appFile << "    \n";
    appFile << "    // Classify hosts by cluster type\n";
    appFile << "    std::vector<sg4::Host*> edge_hosts, fog_hosts, cloud_hosts;\n";
    appFile << "    \n";
    appFile << "    for (auto* host : hosts) {\n";
    appFile << "        std::string name = host->get_cname();\n";
    appFile << "        if (name.find(\"edge\") != std::string::npos) {\n";
    appFile << "            edge_hosts.push_back(host);\n";
    appFile << "        } else if (name.find(\"fog\") != std::string::npos) {\n";
    appFile << "            fog_hosts.push_back(host);\n";
    appFile << "        } else if (name.find(\"cloud\") != std::string::npos) {\n";
    appFile << "            cloud_hosts.push_back(host);\n";
    appFile << "        }\n";
    appFile << "    }\n";
    appFile << "    \n";
    appFile << "    XBT_INFO(\"Detected: %zu edge, %zu fog, %zu cloud hosts\", \n";
    appFile << "             edge_hosts.size(), fog_hosts.size(), cloud_hosts.size());\n";
    appFile << "    \n";
    
    // Deploy actors
    if (edgeClusters > 0) {
        appFile << "    // Deploy Edge devices\n";
        appFile << "    for (auto* host : edge_hosts) {\n";
        appFile << "        host->add_actor(\"edge_device\", EdgeDevice());\n";
        appFile << "    }\n";
        appFile << "    \n";
    }
    
    if (fogClusters > 0) {
        appFile << "    // Deploy Fog nodes\n";
        appFile << "    for (auto* host : fog_hosts) {\n";
        appFile << "        host->add_actor(\"fog_node\", FogNode());\n";
        appFile << "    }\n";
        appFile << "    \n";
    }
    
    if (cloudClusters > 0) {
        appFile << "    // Deploy Cloud servers\n";
        appFile << "    for (auto* host : cloud_hosts) {\n";
        appFile << "        host->add_actor(\"cloud_server\", CloudServer());\n";
        appFile << "    }\n";
        appFile << "    \n";
    }
    
    appFile << "    // Run simulation\n";
    appFile << "    e.run();\n";
    appFile << "    \n";
    appFile << "    XBT_INFO(\"=== Simulation completed ===\");\n";
    appFile << "    XBT_INFO(\"Simulated time: %.2f seconds\", sg4::Engine::get_clock());\n";
    appFile << "    \n";
    appFile << "    return 0;\n";
    appFile << "}\n";
    
    appFile.close();
    std::cout << "Template application generated: " << appFilename << std::endl;
    std::cout << "\nNext steps:\n";
    std::cout << "  1. Edit " << appFilename << " and implement your actor logic\n";
    std::cout << "  2. Add to CMakeLists.txt:\n";
    std::cout << "     add_executable(my_app " << appFilename << ")\n";
    std::cout << "     target_link_libraries(my_app enigma_platform ${SimGrid_LIBRARY})\n";
    std::cout << "  3. Compile: cd build && cmake .. && make\n";
    std::cout << "  4. Run: ./build/my_app " << platformFile << "\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    std::string type = argv[1];
    
    try {
        // Simple host-based platforms
        if (type == "edge" && argc >= 3) {
            int numDevices = std::stoi(argv[2]);
            std::cout << "Generating Edge platform with " << numDevices << " devices...\n";
            
            auto zone = EdgePlatform::createStarTopology(numDevices);
            PlatformGenerator gen;
            gen.generatePlatform("platforms/edge_platform.xml", zone);
            
        } else if (type == "fog" && argc >= 3) {
            int numNodes = std::stoi(argv[2]);
            std::cout << "Generating Fog platform with " << numNodes << " nodes...\n";
            
            auto zone = FogPlatform::createHierarchicalTopology(numNodes);
            PlatformGenerator gen;
            gen.generatePlatform("platforms/fog_platform.xml", zone);
            
        } else if (type == "cloud" && argc >= 3) {
            int numServers = std::stoi(argv[2]);
            std::cout << "Generating Cloud platform with " << numServers << " servers...\n";
            
            auto zone = CloudPlatform::createCluster(numServers);
            PlatformGenerator gen;
            gen.generatePlatform("platforms/cloud_platform.xml", zone);
            
        } else if (type == "hybrid") {
            std::cerr << "The 'hybrid' hierarchical mode is deprecated. Use 'hybrid-cluster-flat'.\n";
            return 1;
        } else if (type == "iot" && argc >= 4) {
            int sensors = std::stoi(argv[2]);
            int actuators = std::stoi(argv[3]);
            
            std::cout << "Generating IoT platform:\n";
            std::cout << "  - Sensors: " << sensors << "\n";
            std::cout << "  - Actuators: " << actuators << "\n";
            
            auto zone = EdgePlatform::createIoTPlatform(sensors, actuators);
            PlatformGenerator gen;
            gen.generatePlatform("platforms/iot_platform.xml", zone);
        
        // Cluster-based platforms
        } else if (type == "edge-cluster" && argc >= 4) {
            int numClusters = std::stoi(argv[2]);
            int nodesPerCluster = std::stoi(argv[3]);
            
            std::cout << "Generating Edge platform with " << numClusters 
                      << " clusters of " << nodesPerCluster << " nodes each...\n";
            
            std::vector<ClusterConfig> clusters;
            for (int i = 0; i < numClusters; i++) {
                clusters.emplace_back("edge_cluster_" + std::to_string(i), 
                                     nodesPerCluster, "1Gf", 1, "125MBps", "50us");
            }
            
            auto zone = PlatformGenerator::createEdgeWithClusters("edge_platform", clusters);
            PlatformGenerator gen;
            gen.generatePlatform("platforms/edge_platform.xml", zone);
            
        } else if (type == "fog-cluster" && argc >= 4) {
            int numClusters = std::stoi(argv[2]);
            int nodesPerCluster = std::stoi(argv[3]);
            
            std::cout << "Generating Fog platform with " << numClusters 
                      << " clusters of " << nodesPerCluster << " nodes each...\n";
            
            std::vector<ClusterConfig> clusters;
            for (int i = 0; i < numClusters; i++) {
                clusters.emplace_back("fog_cluster_" + std::to_string(i), 
                                     nodesPerCluster, "10Gf", 4, "1GBps", "10us");
            }
            
            auto zone = PlatformGenerator::createFogWithClusters("fog_platform", clusters);
            PlatformGenerator gen;
            gen.generatePlatform("platforms/fog_platform.xml", zone);
            
        } else if (type == "cloud-cluster" && argc >= 4) {
            int numClusters = std::stoi(argv[2]);
            int nodesPerCluster = std::stoi(argv[3]);
            
            std::cout << "Generating Cloud platform with " << numClusters 
                      << " clusters of " << nodesPerCluster << " nodes each...\n";
            
            std::vector<ClusterConfig> clusters;
            for (int i = 0; i < numClusters; i++) {
                clusters.emplace_back("cloud_cluster_" + std::to_string(i), 
                                     nodesPerCluster, "100Gf", 16, "10GBps", "1us");
            }
            
            auto zone = PlatformGenerator::createCloudWithClusters("cloud_platform", clusters);
            PlatformGenerator gen;
            gen.generatePlatform("platforms/cloud_platform.xml", zone);
            
        } else if (type == "hybrid-cluster" && argc >= 8) {
            int edgeClusters = std::stoi(argv[2]);
            int edgeNodes = std::stoi(argv[3]);
            int fogClusters = std::stoi(argv[4]);
            int fogNodes = std::stoi(argv[5]);
            int cloudClusters = std::stoi(argv[6]);
            int cloudNodes = std::stoi(argv[7]);
            bool directEdgeCloud = false;
            std::string outputFile = "platforms/hybrid_platform.xml";
            bool generateApp = false;
            int optionalIndex = 8;
            
            // Parse optional arguments
            while (optionalIndex < argc) {
                std::string arg = argv[optionalIndex];
                if (arg == "--generate-app") {
                    generateApp = true;
                    optionalIndex++;
                } else {
                    // Try parse as integer (edge_cloud_direct) or filename
                    char* endPtr = nullptr;
                    long val = strtol(argv[optionalIndex], &endPtr, 10);
                    if (endPtr != nullptr && *endPtr == '\0') {
                        // It's a number - edge_cloud_direct flag
                        directEdgeCloud = (val != 0);
                        optionalIndex++;
                    } else {
                        // It's a filename
                        outputFile = argv[optionalIndex];
                        if (outputFile.find('/') == std::string::npos) {
                            outputFile = std::string("platforms/") + outputFile;
                        }
                        optionalIndex++;
                    }
                }
            }
            
            std::cout << "Generating flat hybrid platform with clusters:\n";
            std::cout << "  - Edge: " << edgeClusters << " clusters × " << edgeNodes << " nodes\n";
            std::cout << "  - Fog: " << fogClusters << " clusters × " << fogNodes << " nodes\n";
            std::cout << "  - Cloud: " << cloudClusters << " clusters × " << cloudNodes << " nodes\n";
            std::cout << "  - Direct Edge-Cloud links: " << (directEdgeCloud ? "ENABLED" : "DISABLED") << "\n";
            std::cout << "  - Output file: " << outputFile << "\n";
            std::cout << "  - Generate template app: " << (generateApp ? "YES" : "NO") << "\n";
            
            std::vector<ClusterConfig> edgeClustersVec, fogClustersVec, cloudClustersVec;
            
            for (int i = 0; i < edgeClusters; i++) {
                edgeClustersVec.emplace_back("edge_cluster_" + std::to_string(i), 
                                            edgeNodes, "1Gf", 1, "125MBps", "50us");
            }
            
            for (int i = 0; i < fogClusters; i++) {
                fogClustersVec.emplace_back("fog_cluster_" + std::to_string(i), 
                                           fogNodes, "10Gf", 4, "1GBps", "10us");
            }
            
            for (int i = 0; i < cloudClusters; i++) {
                cloudClustersVec.emplace_back("cloud_cluster_" + std::to_string(i), 
                                             cloudNodes, "100Gf", 16, "10GBps", "1us");
            }
            
            auto zone = PlatformGenerator::createHybridWithClustersFlat(edgeClustersVec, fogClustersVec, cloudClustersVec, directEdgeCloud);
            PlatformGenerator gen;
            gen.generatePlatform(outputFile, zone);
            
            // Generate template app if requested
            if (generateApp) {
                std::string appFilename = outputFile;
                // Replace .xml with .cpp and move to tests/
                size_t lastSlash = appFilename.find_last_of('/');
                size_t lastDot = appFilename.find_last_of('.');
                std::string baseName = appFilename.substr(lastSlash + 1, lastDot - lastSlash - 1);
                std::string appFile = "tests/" + baseName + "_app.cpp";
                
                generateTemplateApp(appFile, outputFile, 
                                   edgeClusters, edgeNodes, 
                                   fogClusters, fogNodes, 
                                   cloudClusters, cloudNodes);
            }
            
        } else {
            std::cerr << "Error: Invalid arguments\n\n";
            printUsage(argv[0]);
            return 1;
        }
        
        std::cout << "\nPlatform successfully generated!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
