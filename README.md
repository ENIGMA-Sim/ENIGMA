# ENIGMA - gENeric Iot edGe siMulAtor

A C++ project that enables creation of XML platforms for Edge, Fog, and Cloud infrastructures, with applications using SimGrid 4.1 for simulation.

## Requirements

- C++17 or higher
- CMake 3.10 or higher
- SimGrid 4.1 or higher
- Compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)

**Python (optional – for Python simulations and mobility visualization):**

- Python 3.8+
- SimGrid Python bindings (`libsimgrid-dev` includes them)
- `folium` (`pip install folium`) – interactive map generation
- `playwright` (`pip install playwright && playwright install chromium`) – live browser visualization

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
├── include/mobility/        # Mobility module headers (C++)
├── src/mobility/            # Mobility module sources (C++)
├── src/python/              # Python API
│   ├── enigma/mobility/    # Python mobility package
│   ├── tests/              # Python test apps
│   └── tools/              # Post-sim viewer (mobility_viewer.py)
├── platforms/coords/        # GPS trace CSV files (one per device)
├── tests/                   # Test/Example applications
│   ├── edge_computing.cpp  # Basic edge computing
│   ├── fog_analytics.cpp   # Fog analytics
│   ├── hybrid_cloud.cpp    # Multi-tier hybrid
│   ├── data_offloading.cpp # Smart offloading decisions
│   ├── mqtt_edge_app.cpp   # MQTT pub/sub example
│   └── mobility_test.cpp   # Mobility module demo
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

### 4. Use Mobility (Optional)

The mobility module attaches real GPS traces to simulated devices and records their positions during the simulation.  After the simulation it exports the data and generates an **interactive map** (OpenStreetMap) where you can drag a time slider to see each device's position and all its recorded stats at every timestamp.

> 📖 **Full documentation:**
> - [C++ Mobility Module](include/mobility/README.md) — `MobilityPosition`, `MobilityTrace`, `MobilityManager`, build instructions, CLI demo
> - [Python Mobility Module](src/python/enigma/mobility/README.md) — `MobilityManager`, `MobilityRecorder`, `MobilityVisualizer`, standalone viewer, online/offline maps

#### GPS trace format

Create one CSV file per device inside a directory (e.g. `platforms/coords/`).  Only `timestamp`, `latitude` and `longitude` are mandatory.  All other columns (whatever names and however many) are automatically loaded, interpolated, and shown in map popups.

```
platforms/coords/
├── edge_cluster_0_node_0.csv
├── edge_cluster_0_node_1.csv
└── ...
```

```csv
timestamp,latitude,longitude,altitude,speed,heading,accuracy
0,48.8572,2.3540,34.6,5.0,90.0,7.5
1,48.8572,2.3541,34.8,5.0,92.1,7.3
...
```

The filename stem must match the SimGrid host name exactly.

#### Declare the coords directory in the XML platform

Add a `mobility_dir` property to any zone in your platform XML:

```xml
<zone id="my_platform" routing="Full">
  <prop id="mobility_dir" value="platforms/coords/"/>
  <!-- ... rest of platform ... -->
</zone>
```

#### C++ usage

```cpp
#include "mobility/MobilityManager.hpp"
using namespace enigma::mobility;

int main(int argc, char* argv[]) {
    simgrid::s4u::Engine e(&argc, argv);
    e.load_platform(argv[1]);

    // Auto-detects mobility_dir from XML or pass it explicitly:
    MobilityManager mob(e);                        // reads XML property
    // MobilityManager mob(e, "platforms/coords/"); // explicit path

    // Start a periodic actor that records all device positions every 0.5 s
    mob.start_periodic_actor(e, 0.5);

    // Inside any actor: query interpolated position at current time
    auto pos = mob.position_at("edge_cluster_0_node_1",
                               simgrid::s4u::Engine::get_clock());
    if (pos) {
        double spd = pos->extra.count("speed") ? pos->extra.at("speed") : 0.0;
        XBT_INFO("lat=%.6f lon=%.6f  speed=%.1f m/s",
                 pos->latitude, pos->longitude, spd);
    }

    e.run();

    // Export results
    mob.export_json("snapshots.json");
    mob.export_csv("snapshots.csv");
    mob.export_traces_json("raw_traces.json");
    return 0;
}
```

Build with the `enigma_mobility` library:

```cmake
add_executable(my_app tests/my_app.cpp)
target_link_libraries(my_app enigma_mobility enigma_platform ${SimGrid_LIBRARY})
```

#### Python usage

```python
import sys
import simgrid
from enigma.mobility import MobilityManager, MobilityVisualizer

sys.path.insert(0, "src/python")

def my_actor(mob: MobilityManager, host_name: str):
    for _ in range(10):
        t   = simgrid.Engine.clock
        pos = mob.record(host_name, t)       # record + return position
        if pos:
            spd = pos.extra.get("speed", 0.0)
            simgrid.this_actor.info(f"lat={pos.latitude:.6f}  spd={spd:.1f} m/s")
        simgrid.this_actor.sleep_for(1.0)

def main():
    e = simgrid.Engine(sys.argv)
    e.load_platform(sys.argv[1])

    mob = MobilityManager(e, coords_dir="platforms/coords/")
    mob.start_periodic_actor(e, interval_s=0.5)

    for host in e.all_hosts:
        host.add_actor("worker", lambda h=host.name: my_actor(mob, h))

    e.run()

    # Exports
    mob.recorder.export_csv("snapshots.csv")
    mob.recorder.export_json("snapshots.json")
    mob.recorder.export_geojson("trajectories.geojson")
    mob.recorder.print_stats()

    # Interactive map (requires folium: pip install folium)
    MobilityVisualizer.save_interactive_map(mob.recorder, "mobility_map.html")

main()
```

#### Interactive map visualization

After the simulation, open the generated `mobility_map.html` in any browser:

- **Trajectory lines** for every device on OpenStreetMap
- **Time slider** – drag or press Play to animate device positions frame by frame; the slider step matches the recording interval automatically
- **Click any dot** to see a popup with all recorded stats for that device at that timestamp (whatever columns your CSV contained)
- **Layer toggle** and device colour legend

##### Online mode (default)

Playwright opens Chromium directly, which bypasses the `file://` CORS
restrictions that normal browsers enforce.  The map loads CDN resources
(Leaflet, jQuery, etc.) from the internet – no local HTTP server needed.

##### Offline mode (`--offline`)

Downloads and embeds all JS/CSS inline into the HTML file so it works
perfectly as `file://` even without internet access.  Map tiles
(OpenStreetMap background) are still fetched live.  The file will be ~6.5 MB.

```bash
# Online (default) – CDN resources loaded at view time
python3 src/python/tests/mobility_test.py platforms/my_platform.xml

# Offline – everything inlined, opens as file:// without internet
python3 src/python/tests/mobility_test.py platforms/my_platform.xml --offline
```

| Mode | CDN resources | Needs internet to view | HTML file size |
|------|:---:|:---:|:---:|
| `--online` (default) | fetched at view time | ✅ CDN + tiles | ~1.5 MB |
| `--offline` | embedded inline | tiles only | ~6.5 MB |

You can also generate the map from any previously saved snapshot file:

```bash
# From a JSON or CSV snapshot file
python3 src/python/tools/mobility_viewer.py snapshots.json
python3 src/python/tools/mobility_viewer.py snapshots.json --save my_map.html --offline

# Print trajectory statistics without opening a map
python3 src/python/tools/mobility_viewer.py snapshots.csv --stats-only

# Filter specific devices
python3 src/python/tools/mobility_viewer.py snapshots.json \
    --devices edge_cluster_0_node_1 edge_cluster_1_node_2

# C++ output can also be visualized directly
python3 src/python/tools/mobility_viewer.py /tmp/cpp_mob_out_snapshots.json
```

#### Live map during simulation

Instantiate `MobilityVisualizer` before the simulation and pass it to `MobilityManager`.  A Playwright/Chromium window opens automatically and the map refreshes every few seconds.  When the simulation ends the final map is loaded in the same window; the program exits automatically when you close the browser.

```python
viz = MobilityVisualizer(
    title="My Sim – Live",
    live_path="/tmp/mobility_live.html",
    update_interval_s=5.0,
)
viz.start()   # opens Chromium automatically

mob = MobilityManager(e, coords_dir="platforms/coords/", visualizer=viz)
mob.start_periodic_actor(e, 0.5)
e.run()

viz.stop()                                            # keeps browser open
viz.show_final("final_map.html")                      # navigate to final map
viz.wait_for_close()                                  # blocks until user closes browser
```

#### mobility_test.py – available flags

The Python simulation test (`src/python/tests/mobility_test.py`) accepts the following options:

```
python3 src/python/tests/mobility_test.py <platform.xml> [options]
```

| Flag | Default | Description |
|------|---------|-------------|
| `--coords-dir <dir>` | from XML | Override the `mobility_dir` property in the platform XML |
| `--output <dir>` | `tests/mobility_output/` | Directory where all output files are written |
| `--interval <s>` | `0.5` | Snapshot recording interval in simulation seconds |
| `--no-live` | off | Disable the live Playwright browser; only record data and generate files |
| `--no-replay` | off | Skip loading the final map into the browser after the simulation ends |
| `--online` | ✅ default | Map HTML references CDN URLs (loaded at view time) |
| `--offline` | off | Embed all JS/CSS inline so the map works as `file://` without internet |
| `--fps <n>` | `15` | Replay animation frames-per-second |
| `--save-replay` | off | Save the animated replay to `mobility_output/replay.gif` |

**Browser behaviour by flag combination:**

| Command | Browser opens? | Process exits when |
|---------|:---:|--------------------|
| *(default)* | ✅ live during sim + final map | User closes browser or Ctrl+C |
| `--offline` | ✅ same, map embedded | User closes browser or Ctrl+C |
| `--no-replay` | ✅ live during sim only | Simulation ends (browser auto-closes) |
| `--no-live` | ❌ never | Immediately after files are saved |
| `--no-live --no-replay` | ❌ never | Immediately after files are saved |

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
- **mobility_test**: Mobility module demo – loads GPS traces, records snapshots, exports JSON/CSV and interactive map

Python equivalents live in `src/python/tests/`.

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
│   ├── mobility/           # Mobility module (C++)
│   │   ├── MobilityPosition.hpp  # Position snapshot (timestamp+lat+lon+extra)
│   │   ├── MobilityTrace.hpp     # CSV loader + linear interpolation
│   │   └── MobilityManager.hpp   # Manager: load traces, record, export
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
│   ├── mobility/           # Mobility module (C++)
│   │   ├── MobilityTrace.cpp
│   │   └── MobilityManager.cpp
│   ├── tools/              # CLI tools
│   │   └── platform_generator_main.cpp
│   ├── utils/              # Utility implementations
│   │   └── XMLWriter.cpp
│   └── python/             # Python API
│       ├── enigma/
│       │   └── mobility/   # Python mobility package
│       │       ├── __init__.py
│       │       ├── mobility_trace.py      # CSV loader + interpolation
│       │       ├── mobility_manager.py    # SimGrid integration
│       │       ├── mobility_recorder.py   # Snapshot collection + export
│       │       └── mobility_visualizer.py # Folium interactive map
│       ├── tests/
│       │   └── mobility_test.py           # Python simulation demo
│       └── tools/
│           └── mobility_viewer.py         # Standalone post-sim viewer
│
├── tests/                   # Test applications (C++)
│   ├── edge_computing.cpp  # Edge-only processing
│   ├── fog_analytics.cpp   # Fog layer analytics
│   ├── hybrid_cloud.cpp    # Multi-tier application
│   ├── data_offloading.cpp # Smart offloading with responses
│   ├── mqtt_edge_app.cpp   # MQTT pub/sub IoT example
│   └── mobility_test.cpp   # Mobility module demo
│
├── platforms/               # XML platforms
│   └── coords/             # GPS trace CSV files (one per device)
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