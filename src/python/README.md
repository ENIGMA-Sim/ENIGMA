# ENIGMA – Python Port

Python rewrite of the ENIGMA Edge-Fog-Cloud simulation framework using
**SimGrid Python bindings**.  The Python port mirrors every major component of
the original C++ implementation and targets Python ≥ 3.10.

---

## Directory layout

```
src/python/
├── enigma/                     # Main importable package
│   ├── platform/               # Platform generation (XML)
│   │   ├── configs.py          # Data-classes: ZoneConfig, ClusterConfig …
│   │   ├── platform_generator.py  # Core XML writer
│   │   ├── edge_platform.py    # Edge topology factories
│   │   ├── fog_platform.py     # Fog topology factories
│   │   ├── cloud_platform.py   # Cloud topology factories
│   │   └── platform_builder.py # Fluent builder API
│   └── comms/
│       └── mqtt/               # MQTT pub/sub over SimGrid Mailboxes
│           ├── mqtt_broker.py
│           ├── mqtt_publisher.py
│           ├── mqtt_subscriber.py
│           └── mqtt_config.py
├── tools/
│   └── platform_generator_main.py   # CLI platform generator
├── tests/
│   ├── hybrid_cloud.py         # 3-tier Edge→Fog→Cloud simulation
│   ├── mqtt_edge_app.py        # MQTT pub/sub edge simulation
│   ├── edge_computing.py       # Edge devices + gateway
│   ├── fog_analytics.py        # Data sources → Fog analyser
│   └── data_offloading.py      # Adaptive offloading decisions
└── requirements.txt
```

---

## Installation

### 1. Install SimGrid with Python bindings

**From source** (recommended for the latest version):

```bash
git clone https://framagit.org/simgrid/simgrid.git
cd simgrid
cmake -DENABLE_PYTHON=ON -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

**Via pip** (pre-built wheels, not always available for every platform):

```bash
pip install simgrid
```

Verify the installation:

```bash
python3 -c "import simgrid; print('SimGrid OK')"
```

### 2. Clone / use the ENIGMA workspace

All Python files live in `src/python/`.  No additional `pip install` step is
needed beyond SimGrid itself.

---

## Platform generation (no SimGrid required)

The platform generator only uses the Python standard library. You can run it
without an active SimGrid installation to produce XML platform files:

```bash
cd src/python

# Simple platforms
python3 tools/platform_generator_main.py edge  10
python3 tools/platform_generator_main.py fog   5
python3 tools/platform_generator_main.py cloud 4
python3 tools/platform_generator_main.py iot   8 3

# Cluster-based
python3 tools/platform_generator_main.py edge-cluster  3 5
python3 tools/platform_generator_main.py fog-cluster   2 4
python3 tools/platform_generator_main.py cloud-cluster 1 8

# Flat hybrid with optional app template
python3 tools/platform_generator_main.py hybrid-cluster \
    2 10 \   # 2 edge clusters × 10 nodes
    2  5 \   # 2 fog  clusters × 5  nodes
    1 20 \   # 1 cloud cluster × 20 nodes
    0 platforms/my_hybrid.xml --generate-app
```

Generated XML files are written to `platforms/` (created automatically).

---

## Running simulations

Each test script takes a SimGrid platform XML as its first argument:

```bash
cd src/python

# Hybrid 3-tier simulation
python3 tests/hybrid_cloud.py ../../platforms/hybrid_platform.xml

# MQTT edge app
python3 tests/mqtt_edge_app.py ../../platforms/edge_platform.xml

# Edge computing
python3 tests/edge_computing.py ../../platforms/edge_platform.xml

# Fog analytics
python3 tests/fog_analytics.py ../../platforms/hybrid_platform.xml

# Data offloading
python3 tests/data_offloading.py ../../platforms/hybrid_platform.xml
```

---

## Using the library in your own scripts

```python
import sys
import simgrid
from enigma.platform import PlatformGenerator, PlatformBuilder, ClusterConfig

# ── 1. Generate a platform XML ───────────────────────────────────────────────
xml = (
    PlatformBuilder()
    .create_platform("my_platform")
    .add_edge_layer(num_devices=4)
    .add_fog_layer(num_nodes=2)
    .add_cloud_layer(num_servers=1)
    .get_platform_xml()
)
print(xml[:200])

# ── 2. Load the platform and run a simulation ────────────────────────────────
builder = PlatformBuilder().create_platform("sim")
builder.add_edge_layer(2).add_fog_layer(1).add_cloud_layer(1)
builder.build_to_file("/tmp/sim_platform.xml")

e = simgrid.Engine(sys.argv)
e.load_platform("/tmp/sim_platform.xml")

def my_actor():
    simgrid.this_actor.info("Hello from actor!")
    simgrid.this_actor.execute(1e9)

for host in e.all_hosts:
    if "edge" in host.name:
        simgrid.Actor.create("my_actor", host, my_actor)

e.run()
```

---

## Correspondence with the C++ version

| C++ component | Python equivalent |
|---|---|
| `include/platform/PlatformGenerator.hpp` | `enigma/platform/platform_generator.py` |
| `include/platform/EdgePlatform.hpp` | `enigma/platform/edge_platform.py` |
| `include/platform/FogPlatform.hpp` | `enigma/platform/fog_platform.py` |
| `include/platform/CloudPlatform.hpp` | `enigma/platform/cloud_platform.py` |
| `include/platform/PlatformBuilder.hpp` | `enigma/platform/platform_builder.py` |
| `include/comms/mqtt/MQTTBroker.hpp` | `enigma/comms/mqtt/mqtt_broker.py` |
| `include/comms/mqtt/MQTTPublisher.hpp` | `enigma/comms/mqtt/mqtt_publisher.py` |
| `include/comms/mqtt/MQTTSubscriber.hpp` | `enigma/comms/mqtt/mqtt_subscriber.py` |
| `src/tools/platform_generator_main.cpp` | `tools/platform_generator_main.py` |
| `tests/hybrid_cloud.cpp` | `tests/hybrid_cloud.py` |
| `tests/mqtt_edge_app.cpp` | `tests/mqtt_edge_app.py` |
| `tests/edge_computing.cpp` | `tests/edge_computing.py` |
| `tests/fog_analytics.cpp` | `tests/fog_analytics.py` |
| `tests/data_offloading.cpp` | `tests/data_offloading.py` |

---

## SimGrid Python API quick reference

```python
import simgrid

# Engine
e = simgrid.Engine(sys.argv)
e.load_platform("platform.xml")
e.run()
hosts = e.all_hosts          # list[simgrid.Host]
clock = simgrid.Engine.clock

# Actors
simgrid.Actor.create("name", host, callable_or_function)

# Inside an actor
host   = simgrid.this_actor.get_host()
simgrid.this_actor.sleep_for(seconds)
simgrid.this_actor.execute(flops)
simgrid.this_actor.info("message")
simgrid.this_actor.warning("warn")

# Mailboxes (point-to-point message passing)
mbox = simgrid.Mailbox.by_name("mbox_name")
mbox.put(python_object, byte_size)
obj  = mbox.get()            # blocking receive
```
