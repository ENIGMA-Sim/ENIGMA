# ENIGMA - gENeric Iot edGe siMulAtor

A C++ project that enables creation of XML platforms for Edge, Fog, and Cloud infrastructures, with applications using SimGrid 4.1 for simulation.

## Requirements

- C++17 or higher
- CMake 3.10 or higher
- SimGrid 4.1 or higher
- Compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

## SimGrid Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install simgrid libsimgrid-dev
```

### From Source
```bash
git clone https://github.com/simgrid/simgrid.git
cd simgrid
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/opt/simgrid-4.1 ..
make -j$(nproc)
sudo make install
```

## Build

```bash
mkdir build
cd build
cmake ..
make
```

## Project Structure

```
ENIGMA/
├── include/                 # Public headers
│   ├── platform/           # Platform generators (Edge, Fog, Cloud)
│   ├── comms/              # Communication modules
│   │   └── mqtt/           # MQTT headers (Broker, Publisher, Subscriber)
│   └── utils/              # Utility headers (XMLWriter)
├── src/                     # Implementation files
│   ├── platform/           # Platform generator implementations
│   ├── comms/              # Communication module implementations
│   │   └── mqtt/           # MQTT implementation (Broker, Publisher, Subscriber)
│   ├── tools/              # Command-line tools (platform_generator)
│   └── utils/              # Utility implementations (XMLWriter)
├── tests/                   # Test/Example applications
│   ├── edge_computing.cpp  # Basic edge computing
│   ├── fog_analytics.cpp   # Fog analytics
│   ├── hybrid_cloud.cpp    # Multi-tier hybrid
│   ├── data_offloading.cpp # Smart offloading decisions
│   ├── mqtt_edge_app.cpp   # MQTT pub/sub example
├── build/                   # Build artifacts (generated)
├── CMakeLists.txt           # CMake configuration
├── build.sh                 # Build script
├── run_examples.sh          # Interactive examples runner
└── README.md                
```

## Usage

### 1. Generate a Platform

```cpp
#include "platform/PlatformBuilder.hpp"

int main() {
    PlatformBuilder builder;
    
    // Create an Edge-Fog-Cloud platform
    builder.createEdgeFogCloud("my_platform.xml")
           .addEdgeLayer(10, "1Gf", "125MBps")
           .addFogLayer(5, "10Gf", "1GBps")
           .addCloudLayer(3, "100Gf", "10GBps")
           .build();
    
    return 0;
}
```

### 2. Run an Application

```cpp
#include <simgrid/s4u.hpp>

int main(int argc, char* argv[]) {
    simgrid::s4u::Engine e(&argc, argv);
    e.load_platform("platforms/my_platform.xml");
    e.load_deployment("deployments/my_deployment.xml");
    e.run();
    
    return 0;
}
```

### 3. Use MQTT (Optional)

```cpp
#include "comms/mqtt/MQTT.hpp"

void sensor_actor() {
    MQTTPublisher pub("mqtt_broker");
    pub.publish("sensors/temp", "25.5°C", 100);
}

void gateway_actor() {
    MQTTSubscriber sub("mqtt_broker");
    sub.subscribe("sensors/temp");
    auto msg = sub.receive();
}
```

## Platform Generator Tool

The `platform_generator` CLI tool allows you to quickly generate hybrid Edge-Fog-Cloud platforms:

### Basic Usage

```bash
./build/platform_generator hybrid-cluster <eC> <eN> <fC> <fN> <cC> <cN>
```

Where:
- `eC`: Number of Edge clusters
- `eN`: Nodes per Edge cluster
- `fC`: Number of Fog clusters
- `fN`: Nodes per Fog cluster
- `cC`: Number of Cloud clusters
- `cN`: Nodes per Cloud cluster

**Example:**
```bash
# Generate platform with 2 Edge clusters (5 nodes each), 1 Fog cluster (3 nodes), 1 Cloud cluster (10 nodes)
./build/platform_generator hybrid-cluster 2 5 1 3 1 10
```

### Advanced Options

#### Direct Edge-Cloud Connectivity

Add `1` as the 7th parameter to enable direct Edge-to-Cloud links (bypassing Fog):

```bash
./build/platform_generator hybrid-cluster 2 5 1 3 1 10 1
```

This creates 2GBps/30ms links between Edge and Cloud clusters in addition to the normal Edge→Fog→Cloud topology.

#### Custom Output Filename

Specify a custom filename for the generated platform:

```bash
./build/platform_generator hybrid-cluster 2 5 1 3 1 10 1 my_custom_platform.xml
```

The tool automatically prepends `platforms/` if you don't specify a path.

#### Generate Template Application

Use the `--generate-app` flag to automatically create a C++ template application:

```bash
./build/platform_generator hybrid-cluster 2 5 1 3 1 10 1 my_platform.xml --generate-app
```

This generates:
- **Platform XML**: `platforms/my_platform.xml`
- **Template App**: `tests/my_platform_app.cpp`

The template includes:
- `EdgeDevice` class (if edge clusters > 0)
- `FogNode` class (if fog clusters > 0)
- `CloudServer` class (if cloud clusters > 0)
- Actor deployment logic with host classification
- TODO sections for implementing your custom logic

**To use the generated template:**

1. **Edit the template** and implement your logic in the TODO sections
2. **Add to CMakeLists.txt**:
   ```cmake
   add_executable(my_platform_app tests/my_platform_app.cpp)
   target_link_libraries(my_platform_app enigma_platform ${SimGrid_LIBRARY})
   ```
3. **Rebuild and run**:
   ```bash
   cd build && make my_platform_app
   ./my_platform_app ../platforms/my_platform.xml
   ```

### Complete Example

```bash
# Generate a complete Edge-Fog-Cloud platform with direct Edge-Cloud links and template app
./build/platform_generator hybrid-cluster 3 10 2 5 1 20 1 iot_platform.xml --generate-app

# Output:
#   - platforms/iot_platform.xml
#   - tests/iot_platform_app.cpp
```

## Examples

The `tests/` directory contains complete example applications:

- **edge_computing**: Edge application with distributed processing
- **fog_analytics**: Analytics system with Fog nodes
- **hybrid_cloud**: Hybrid Edge-Fog-Cloud architecture
- **data_offloading**: Smart offloading with request/response cycle
- **mqtt_edge_app**: MQTT publish/subscribe pattern for IoT/Edge

## 📁 Detailed Project Structure

```
ENIGMA/
├── include/                 # Public API headers
│   ├── platform/           # Platform generation
│   │   ├── PlatformGenerator.hpp
│   │   ├── PlatformBuilder.hpp
│   │   ├── EdgePlatform.hpp
│   │   ├── FogPlatform.hpp
│   │   └── CloudPlatform.hpp
│   ├── comms/              # Communication protocols
│   │   └── mqtt/           # MQTT module
│   │       ├── MQTT.hpp           # Convenience header
│   │       ├── MQTTBroker.hpp     # Broker component
│   │       ├── MQTTPublisher.hpp  # Publisher client
│   │       └── MQTTSubscriber.hpp # Subscriber client
│   └── utils/              # Utilities
│       └── XMLWriter.hpp
│
├── src/                     # Implementation files
│   ├── platform/           # Platform implementations
│   │   ├── PlatformGenerator.cpp
│   │   ├── PlatformBuilder.cpp
│   │   ├── EdgePlatform.cpp
│   │   ├── FogPlatform.cpp
│   │   └── CloudPlatform.cpp
│   ├── comms/              # Communication implementations
│   │   └── mqtt/           # MQTT implementation
│   │       ├── MQTTBroker.cpp
│   │       ├── MQTTPublisher.cpp
│   │       └── MQTTSubscriber.cpp
│   ├── tools/              # CLI tools
│   │   └── platform_generator_main.cpp
│   └── utils/              # Utility implementations
│       └── XMLWriter.cpp
│
├── tests/                   # Test applications (5 apps)
│   ├── edge_computing.cpp  # Edge-only processing
│   ├── fog_analytics.cpp   # Fog layer analytics
│   ├── hybrid_cloud.cpp    # Multi-tier application
│   ├── data_offloading.cpp # Smart offloading with responses
│   ├── mqtt_edge_app.cpp   # MQTT pub/sub IoT example
│
├── platforms/               # Generated XML platforms
├── deployments/             # Deployment configurations
├── build/                   # Build output directory
│
├── CMakeLists.txt           # Main build configuration
├── build.sh                 # Quick build script
├── run_examples.sh          # Interactive runner
├── verify.sh                # Verification script
│
├── QUICKSTART.md            # Quick start guide
├── CLUSTER_USAGE.md         # Cluster generation guide
└── README.md                
```


## Publications

### 2023

<details>
<summary>:newspaper: A Scalable Simulator for Cloud, Fog and Edge Computing platforms with Mobility Support.</summary>
 
  * Authors: Elías Del-Pozo-Puñal, Félix García-Carballeira, Diego Camarmas-Alonso
  * Journal Paper: Future Generation Computer Systems
  * Link: [:link: Open publication](https://www.sciencedirect.com/science/article/pii/S0167739X23000511)
  * Cite:
  ```bash
@article{DELPOZOPUNAL2023117,
title = {A Scalable Simulator for Cloud, Fog and Edge Computing platforms with Mobility Support},
journal = {Future Generation Computer Systems},
volume = {144},
pages = {117-130},
year = {2023},
issn = {0167-739X},
doi = {https://doi.org/10.1016/j.future.2023.02.010},
url = {https://www.sciencedirect.com/science/article/pii/S0167739X23000511}
}
  ```
</details>

<details>
<summary>:newspaper: ENIGMA: A Scalable Simulator for IoT and Edge Computing.</summary>
 
  * Authors: Elías Del-Pozo-Puñal, Félix García-Carballeira, Diego Camarmas-Alonso
  * Conference Paper: Parallel, Distributed, and Network-Based Processing (PDP23) [CORE: C]
  * Link: [:link: Open publication](https://zenodo.org/records/15476670)
  * Cite:
  ```bash
@misc{del_pozo_punal_2025_15476670,
  author       = {Del Pozo Puñal, Elías and Garcia-Carballeira, Felix and Camarmas Alonso, Diego},
  title        = {ENIGMA: A Scalable Simulator for IoT and Edge Computing},
  month        = may,
  year         = 2025,
  publisher    = {Zenodo},
  doi          = {10.5281/zenodo.15476670},
  url          = {https://doi.org/10.5281/zenodo.15476670},
}
  ```
</details>

### 2022

<details>
<summary>:newspaper: A Proposal of Mobility Support for the SimGrid Toolkit: Application to IoT simulations.</summary>
 
  * Authors: Elías Del-Pozo-Puñal, Félix García-Carballeira, Diego Camarmas-Alonso
  * Conference Paper: 30th Euromicro International Conference on Parallel, Distributed and Network-based Processing (PDP) [CORE: C]
  * Link: [:link: Open publication](https://ieeexplore.ieee.org/document/9756681/)
  * Cite:
  ```bash



  ```
</details>

<details>
<summary>:newspaper: Una propuesta para el uso de dispositivos móviles en SimGrid: Aplicación a las simulaciones de IoT.</summary>
 
  * Authors: Elías Del-Pozo-Puñal, Félix García-Carballeira, Diego Camarmas-Alonso
  * Conference Paper: XXXII Jornadas del Paralelismo (JP21/22)
  * Link: [:link: Open publication](https://doi.org/10.5281/zenodo.10706513)
  * Cite:
  ```bash
@misc{del_pozo_punal_2022_10706513,
  author       = {Del-Pozo-Puñal, Elías and
                  Garcia-Carballeira, Felix and
                  Camarmas-Alonso, Diego},
  title        = {Una propuesta para el uso de dispositivos móviles
                   en SimGrid: Aplicación a las simulaciones de IoT
                  },
  month        = jul,
  year         = 2022,
  publisher    = {Zenodo},
  version      = {Version v1},
  doi          = {10.5281/zenodo.10706513},
  url          = {https://doi.org/10.5281/zenodo.10706513},
}
  ```
</details>

### 2021

<details>
<summary>:newspaper: ENIGMA: simulador de plataformas Edge y Fog Computing.</summary>
 
  * Authors: Elías Del-Pozo-Puñal, Félix García-Carballeira
  * Conference Paper: Congreso Español de Informática (CEDI 20/21)
  * Link: [:link: Open publication](https://doi.org/10.5281/zenodo.10953027)
  * Cite:
  ```bash
@misc{del_pozo_punal_2021_10953027,
  author       = {Del Pozo Puñal, Elías and
                  Garcia-Carballeira, Felix},
  title        = {ENIGMA: simulador de plataformas Edge y Fog
                   Computing
                  },
  month        = sep,
  year         = 2021,
  publisher    = {Zenodo},
  version      = {Version 1.0},
  doi          = {10.5281/zenodo.10953027},
  url          = {https://doi.org/10.5281/zenodo.10953027},
}
  ```

  <details>
<summary>:newspaper: A generic simulator for Edge Computing platforms.</summary>
 
  * Authors: Elías Del-Pozo-Puñal, Félix García-Carballeira
  * Conference Paper: Conference on High Performance Computing and Simulation (HPCS 2020) [CORE: B]
  * Link: [:link: Open publication](https://doi.org/10.5281/zenodo.10952891)
  * Cite:
  ```bash
@misc{del_pozo_punal_2021_10952891,
  author       = {Del-Pozo-Puñal, Elías and
                  Garcia-Carballeira, Felix},
  title        = {A generic simulator for Edge Computing platforms},
  month        = mar,
  year         = 2021,
  publisher    = {Zenodo},
  version      = {Version 1.0},
  doi          = {10.5281/zenodo.10952891},
  url          = {https://doi.org/10.5281/zenodo.10952891},
}
  ```