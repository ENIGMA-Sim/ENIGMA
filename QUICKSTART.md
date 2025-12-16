# ENIGMA Platform Generator - Quick Start Guide

### Step 1: Install Dependencies

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y simgrid libsimgrid-dev cmake build-essential

# Verify
simgrid_version
```

### Step 2: Build Project

```bash
cd $PATH_TO_ENIGMA/ENIGMA
chmod +x build.sh
./build.sh
```

### Step 3: Generate Your First Platform

```bash
# Create hybrid Edge-Fog-Cloud platform
./build/platform_generator hybrid 10 5 3
```

Creates `platforms/hybrid_platform.xml` with 10 Edge devices, 5 Fog nodes, 3 Cloud servers.

### Step 4: Run Application

```bash
./build/hybrid_cloud_app platforms/hybrid_platform.xml
```

## 📝 Platform Generation Examples

```bash
# Edge platform
./build/platform_generator edge 10

# Fog platform
./build/platform_generator fog 5

# Cloud platform
./build/platform_generator cloud 20

# IoT platform
./build/platform_generator iot 30 10
```

## 🚀 Run Applications

```bash
./build/edge_computing_app platforms/edge_platform.xml
./build/fog_analytics_app platforms/fog_platform.xml
./build/hybrid_cloud_app platforms/hybrid_platform.xml
./build/data_offloading_app platforms/hybrid_platform.xml
```

## 🛠️ Create Your Own Application

```cpp
#include <simgrid/s4u.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(my_app, "My Application");

class MyWorker {
public:
    void operator()() {
        auto* host = simgrid::s4u::this_actor::get_host();
        XBT_INFO("Worker on %s", host->get_cname());
        simgrid::s4u::this_actor::execute(1e9); // 1 GFlop
        XBT_INFO("Work completed");
    }
};

int main(int argc, char* argv[]) {
    simgrid::s4u::Engine e(&argc, argv);
    if (argc < 2) {
        XBT_CRITICAL("Usage: %s <platform.xml>", argv[0]);
        return 1;
    }
    e.load_platform(argv[1]);
    for (auto* host : e.get_all_hosts()) {
        simgrid::s4u::Actor::create("worker", host, MyWorker());
    }
    e.run();
    return 0;
}
```

## 🎯 Create Platform Programmatically

```cpp
#include "platform/PlatformBuilder.hpp"

int main() {
    enigma::PlatformBuilder builder;
    builder.createEdgeFogCloud("custom")
           .addEdgeLayer(15, "2Gf", "200MBps")
           .addFogLayer(8, "15Gf", "2GBps")
           .addCloudLayer(5, "150Gf", "20GBps")
           .build();
    return 0;
}
```

## ❓ Troubleshooting

### Build error
```bash
rm -rf build
./build.sh
```