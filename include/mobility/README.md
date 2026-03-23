# ENIGMA Mobility Module — C++

Attaches real GPS traces to SimGrid hosts, interpolates positions during the
simulation, and exports results for post-simulation visualisation.

## Files

| Header | Purpose |
|--------|---------|
| `MobilityPosition.hpp` | Single geo-position snapshot (`timestamp`, `lat`, `lon`, `extra`) |
| `MobilityTrace.hpp` | CSV loader + linear interpolation for one device |
| `MobilityManager.hpp` | Central manager: load all traces, query positions, record snapshots, export |

Sources are in `src/mobility/`.

---

## Quick-start

### 1 — CSV trace format

One file per device, filename stem must match the SimGrid host name exactly.

```
platforms/coords/
├── edge_0.csv
├── edge_1.csv
└── ...
```

```csv
timestamp,latitude,longitude,speed,heading
0,48.8572,2.3540,5.0,90.0
10,48.8575,2.3543,5.1,91.2
...
```

Only `timestamp`, `latitude` / `lat`, and `longitude` / `lon` / `lng` are
required.  Any extra columns (whatever names, however many) are loaded
automatically, linearly interpolated between waypoints, and included in the
JSON/CSV exports.  Non-numeric extra columns (e.g. `stop_name`) are silently
skipped per row — they do not cause the trace to fail loading.

### 2 — Declare the coords directory in the platform XML

```xml
<zone id="root" routing="Full">
  <prop id="mobility_dir" value="platforms/coords/"/>
  <!-- ... hosts, links ... -->
</zone>
```

The path is relative to the **working directory** when the binary runs.

### 3 — Minimal application

```cpp
#include <simgrid/s4u.hpp>
#include "mobility/MobilityManager.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(my_app, "My app");
namespace sg4 = simgrid::s4u;
using namespace enigma::mobility;

void mobile_actor(MobilityManager* mob) {
    for (int i = 0; i < 20; ++i) {
        double t   = sg4::Engine::get_clock();
        auto   pos = mob->position_at(sg4::this_actor::get_host()->get_cname(), t);
        if (pos) {
            double spd = pos->extra.count("speed") ? pos->extra.at("speed") : 0.0;
            XBT_INFO("t=%.1f  lat=%.6f  lon=%.6f  speed=%.1f m/s",
                     t, pos->latitude, pos->longitude, spd);
        }
        sg4::this_actor::sleep_for(1.0);
    }
}

int main(int argc, char** argv) {
    sg4::Engine e(&argc, argv);
    e.load_platform(argv[1]);                   // XML must have mobility_dir prop

    MobilityManager mob(e);                     // reads mobility_dir from XML
    // MobilityManager mob(e, "my/coords/");    // or pass path explicitly

    XBT_INFO("Traces loaded: %zu", mob.trace_count());

    mob.start_periodic_actor(e, 0.5);          // record every 0.5 sim-seconds

    for (auto* host : e.get_all_hosts()) {
        if (mob.has_trace(host->get_cname()))
            host->add_actor("mobile", mobile_actor, &mob);
    }

    e.run();

    mob.export_json("snapshots.json");
    mob.export_csv("snapshots.csv");
    mob.export_traces_json("raw_traces.json");  // full waypoints, not just snapshots
    return 0;
}
```

### 4 — Build

```cmake
add_executable(my_app tests/my_app.cpp)
target_link_libraries(my_app enigma_mobility enigma_platform ${SimGrid_LIBRARY})
```

```bash
cd build && make my_app -j$(nproc)
./my_app ../platforms/my_platform.xml
```

### 5 — Visualise output

The JSON and CSV files produced by the C++ binary can be fed directly to the
Python viewer (no re-simulation needed):

```bash
# Online mode (Playwright opens as file:// directly — no HTTP server needed)
python3 src/python/tools/mobility_viewer.py snapshots.json

# Offline mode (CDN embedded inline, ~6.5 MB)
python3 src/python/tools/mobility_viewer.py snapshots.json --offline --save map.html
```

---

## API reference

### `MobilityPosition`

```cpp
struct MobilityPosition {
    double timestamp;                        // simulation seconds
    double latitude;                         // decimal degrees
    double longitude;                        // decimal degrees
    std::map<std::string, double> extra;     // all other CSV columns

    double distance_to(const MobilityPosition& other) const; // metres (Haversine)
};
```

### `MobilityTrace`

```cpp
// Load one CSV file
MobilityTrace trace("coords/edge_0.csv");

trace.device_name();          // "edge_0"
trace.size();                 // number of waypoints
trace.t_min(); trace.t_max(); // time range

// Linear interpolation at any simulation time
MobilityPosition pos = trace.position_at(3.7);
```

### `MobilityManager`

```cpp
// Construction
MobilityManager mob(engine);                    // auto-detects mobility_dir from XML
MobilityManager mob(engine, "platforms/coords/"); // explicit path

// Query
mob.trace_count();                              // number of devices loaded
mob.has_trace("edge_0");                        // bool
auto pos = mob.position_at("edge_0", t);        // std::optional<MobilityPosition>

// Recording
mob.record_all(t);                             // snapshot all devices at time t
mob.record("edge_0", t);                       // snapshot one device

// Periodic actor (deploy before e.run())
mob.start_periodic_actor(engine, 0.5);         // every 0.5 sim-seconds

// Access snapshots
const auto& snaps = mob.snapshots();           // vector<Snapshot>

// Export (call after e.run())
mob.export_json("out.json");
mob.export_csv("out.csv");
mob.export_traces_json("raw.json");            // full waypoints (not just recorded snaps)
```

---

## Run the demo

```bash
# Build
cd build && make mobility_test_app -j$(nproc) && cd ..

# Run with the coords2 example platform (9 buses + 15 substations)
./build/mobility_test_app platforms/coords2_platform.xml /tmp/cpp_out

# With explicit coords dir (overrides XML prop)
./build/mobility_test_app platforms/coords2_platform.xml /tmp/cpp_out \
    --mobility-dir platforms/coords2/

# Visualize
python3 src/python/tools/mobility_viewer.py /tmp/cpp_out_snapshots.json --offline
```

Output files:
- `/tmp/cpp_out_snapshots.json` — recorded snapshots (array of objects)
- `/tmp/cpp_out_snapshots.csv`  — same data as CSV
- `/tmp/cpp_out_raw_traces.json` — full raw waypoints per device
