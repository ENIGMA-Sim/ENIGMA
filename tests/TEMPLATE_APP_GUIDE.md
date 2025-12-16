# Template Application Guide

This guide explains how to use the automatically generated template applications from the `platform_generator` tool.

## Overview

When you use the `--generate-app` flag with `platform_generator`, the tool automatically creates a C++ template application skeleton with:
- Actor classes for each tier (Edge/Fog/Cloud) that exists in your platform
- Host classification logic based on naming patterns
- Actor deployment code
- TODO sections where you implement your custom logic

## Quick Start

### 1. Generate Platform and Template

```bash
./build/platform_generator hybrid-cluster 2 10 1 5 1 20 1 my_app.xml --generate-app
```

This creates:
- `platforms/my_app.xml` - The platform description
- `tests/my_app_app.cpp` - Your template application

### 2. Edit the Template

Open `tests/my_app_app.cpp` and look for the `TODO` sections:

```cpp
class EdgeDevice {
public:
    void operator()() {
        const std::string& name = simgrid::s4u::this_actor::get_host()->get_name();
        XBT_INFO("[EDGE] Device '%s' started", name.c_str());
        
        // TODO: Implement edge device logic here
        // Example:
        // - Sense data from environment
        // - Process data locally
        // - Send results to fog/cloud
        // - Use mailboxes for communication:
        //   auto* mbox = simgrid::s4u::Mailbox::by_name("fog_0");
        //   mbox->put(new std::string("sensor_data"), 1000);
        
        XBT_INFO("[EDGE] Device '%s' finished", name.c_str());
    }
};
```

### 3. Add to CMakeLists.txt

Add these lines to `CMakeLists.txt` (around line 115, after other test apps):

```cmake
# Your generated app
add_executable(my_app_app
    tests/my_app_app.cpp
)
target_link_libraries(my_app_app enigma_platform ${SimGrid_LIBRARY})
```

### 4. Build and Run

```bash
cd build
cmake ..
make my_app_app
./my_app_app ../platforms/my_app.xml
```

## Template Structure

The generated template follows this structure:

### Actor Classes

Each tier gets its own actor class (if clusters > 0):

- **EdgeDevice**: Deployed on `edge_cluster_*_node_*` hosts
- **FogNode**: Deployed on `fog_cluster_*_node_*` hosts  
- **CloudServer**: Deployed on `cloud_cluster_*_node_*` hosts

### Main Function

The main function:
1. Loads the platform XML
2. Classifies hosts by name pattern
3. Deploys actors on appropriate hosts
4. Runs the simulation
5. Reports simulation time

## Communication Patterns

### Mailbox Communication

SimGrid uses mailboxes for actor communication:

```cpp
// Sender (e.g., in EdgeDevice)
auto* mbox = simgrid::s4u::Mailbox::by_name("fog_gateway");
auto* payload = new std::string("sensor_data");
mbox->put(payload, 1000); // 1000 bytes

// Receiver (e.g., in FogNode)
auto* mbox = simgrid::s4u::Mailbox::by_name("fog_gateway");
auto* msg = mbox->get<std::string>();
XBT_INFO("Received: %s", msg->c_str());
delete msg;
```

### Request-Response Pattern

```cpp
// EdgeDevice - Send request
auto* request_mbox = simgrid::s4u::Mailbox::by_name("cloud_service");
auto* response_mbox = simgrid::s4u::Mailbox::by_name("edge_" + my_id);
request_mbox->put(new Request{data, response_mbox}, size);

// CloudServer - Process and respond
auto* request = request_mbox->get<Request>();
process(request->data);
request->response_mbox->put(new Response{result}, size);
```

### Using MQTT (Optional)

If your application needs MQTT:

```cpp
#include "comms/mqtt/MQTT.hpp"

// In EdgeDevice
void operator()() {
    MQTTPublisher pub("mqtt_broker");
    pub.publish("sensors/temperature", "25.5", 100);
}

// In FogNode
void operator()() {
    MQTTSubscriber sub("mqtt_broker");
    sub.subscribe("sensors/temperature");
    auto msg = sub.receive();
}
```

Don't forget to link with `enigma_mqtt`:
```cmake
target_link_libraries(my_app_app enigma_platform enigma_mqtt ${SimGrid_LIBRARY})
```

## Common Patterns

### Data Processing Pipeline

```cpp
// EdgeDevice: Sense and send
void operator()() {
    for (int i = 0; i < 10; i++) {
        double sensor_value = sense_environment();
        auto* mbox = simgrid::s4u::Mailbox::by_name("fog_aggregator");
        mbox->put(new double(sensor_value), sizeof(double));
        simgrid::s4u::this_actor::sleep_for(1.0); // 1 second interval
    }
}

// FogNode: Aggregate and forward
void operator()() {
    std::vector<double> values;
    for (int i = 0; i < 10; i++) {
        auto* mbox = simgrid::s4u::Mailbox::by_name("fog_aggregator");
        double* value = mbox->get<double>();
        values.push_back(*value);
        delete value;
    }
    
    double avg = compute_average(values);
    auto* cloud_mbox = simgrid::s4u::Mailbox::by_name("cloud_analytics");
    cloud_mbox->put(new double(avg), sizeof(double));
}

// CloudServer: Analyze
void operator()() {
    auto* mbox = simgrid::s4u::Mailbox::by_name("cloud_analytics");
    double* result = mbox->get<double>();
    XBT_INFO("Analytics result: %f", *result);
    delete result;
}
```

### Load Balancing

```cpp
// FogNode: Round-robin distribution to cloud
void operator()() {
    int cloud_count = 5; // Number of cloud servers
    int next_server = 0;
    
    for (int i = 0; i < 100; i++) {
        auto* task = receive_from_edge();
        
        std::string mbox_name = "cloud_" + std::to_string(next_server);
        auto* mbox = simgrid::s4u::Mailbox::by_name(mbox_name);
        mbox->put(task, task_size);
        
        next_server = (next_server + 1) % cloud_count;
    }
}
```

### Smart Offloading

```cpp
// EdgeDevice: Decide where to process
void operator()() {
    double complexity = analyze_task();
    
    if (complexity < THRESHOLD) {
        // Process locally
        process_on_edge();
    } else {
        // Offload to fog/cloud
        auto* mbox = simgrid::s4u::Mailbox::by_name(
            complexity < HIGH_THRESHOLD ? "fog_service" : "cloud_service"
        );
        mbox->put(new Task{data}, size);
    }
}
```

## Tips and Best Practices

1. **Memory Management**: Always `delete` messages received via mailboxes
2. **Timing**: Use `simgrid::s4u::Engine::get_clock()` to measure simulation time
3. **Logging**: Use `XBT_INFO`, `XBT_DEBUG`, `XBT_WARN` for SimGrid logging
4. **Sleep**: Use `simgrid::s4u::this_actor::sleep_for(seconds)` instead of real sleep
5. **Host Info**: Get current host with `simgrid::s4u::this_actor::get_host()`
6. **Computation**: Simulate computation with `simgrid::s4u::this_actor::execute(flops)`

## Example Workflow

```bash
# 1. Generate platform with 3 edge, 2 fog, 1 cloud clusters
./build/platform_generator hybrid-cluster 3 5 2 3 1 10 1 iot_app.xml --generate-app

# 2. Edit tests/iot_app_app.cpp
vim tests/iot_app_app.cpp

# 3. Add to CMakeLists.txt (uncomment and rename the template section)
vim CMakeLists.txt

# 4. Build
cd build
cmake ..
make iot_app_app

# 5. Run
./iot_app_app ../platforms/iot_app.xml
```

## Troubleshooting

### "Unable to open platform file"
```bash
# Make sure to use relative path from build/ directory
./my_app_app ../platforms/my_app.xml
```

### "Mailbox not found"
- Ensure sender and receiver use the same mailbox name
- Check for typos in mailbox names

### "Segmentation fault"
- Check for memory leaks (missing `delete`)
- Verify mailbox messages are the expected type
- Ensure actors don't access deleted/invalid data

### "No actors running"
- Verify host classification logic matches platform naming
- Check that clusters were created (count > 0)
- Ensure actor classes are being deployed

## Next Steps

- Review existing examples in `tests/`: `edge_computing.cpp`, `fog_analytics.cpp`, `hybrid_cloud.cpp`
- Check SimGrid documentation: https://simgrid.org/doc/latest/
- Explore MQTT integration with `mqtt_edge_app.cpp` example
- Experiment with different platform configurations using `platform_generator`

---

For more information, see:
- `README.md` - Project overview
- `QUICKSTART.md` - Quick start guide  
- `CLUSTER_USAGE.md` - Cluster generation guide
