# Cluster-Based Platform Generation

## Overview

ENIGMA now supports cluster-based platform generation using **native SimGrid `<cluster>` tags**, allowing you to organize hosts into logical clusters. Users can specify the number of clusters and nodes per cluster for each level (edge, fog, cloud).

## Features

- **Native SimGrid Clusters**: Uses SimGrid's native `<cluster>` XML tags for optimal performance
- **Automatic Interconnection**: Clusters are automatically connected using `<zoneRoute>` elements
- **Hierarchical Structure**: Each cluster is wrapped in its own zone for proper routing
- **Flexible Configuration**: Specify different cluster sizes for each level
- **Full Compatibility**: Works seamlessly with SimGrid's routing algorithms

## Command-Line Usage

### Single-Level Platforms with Clusters ✅ PRODUCTION READY

Generate edge platform with 3 clusters of 5 nodes each:
```bash
./build/platform_generator edge-cluster 3 5
```

Generate fog platform with 4 clusters of 8 nodes each:
```bash
./build/platform_generator fog-cluster 4 8
```

Generate cloud platform with 2 clusters of 20 nodes each:
```bash
./build/platform_generator cloud-cluster 2 20
```

### Hybrid Platforms with Clusters

#### Option 1: Flat Hierarchy ✅ RECOMMENDED
**Status: PRODUCTION READY** - Tested and stable

```bash
./build/platform_generator hybrid-cluster-flat <edge_clusters> <nodes_per_edge_cluster> \
                                                <fog_clusters> <nodes_per_fog_cluster> \
                                                <cloud_clusters> <nodes_per_cloud_cluster>
```

Example:
```bash
# 2 edge clusters × 10 nodes, 2 fog clusters × 5 nodes, 1 cloud cluster × 20 nodes
./build/platform_generator hybrid-cluster-flat 2 10 2 5 1 20

# Run hybrid application
./build/hybrid_cloud_app ./platforms/hybrid_platform.xml
```

#### Option 2: 3-Level Hierarchy

**Status: EXPERIMENTAL** - May have routing issues in SimGrid 4.1

```bash
./build/platform_generator hybrid-cluster 2 10 2 5 1 20
```

## Implementation Details

### How It Works

1. **Cluster Definition**: Users specify clusters using simple syntax (number of clusters × nodes per cluster)
2. **Native SimGrid Clusters**: Each cluster is created using SimGrid's `<cluster>` tag
3. **Zone Wrapping**: Each cluster is wrapped in its own zone with "Cluster" routing
4. **Automatic zoneRoutes**: Inter-cluster communication is enabled via `<zoneRoute>` elements with router gateways

### XML Structure Example

For `edge-cluster 3 5`, the generated XML structure is:

```xml
<platform version="4.1">
  <zone id="edge_platform" routing="Full">
    <!-- First cluster in its own zone -->
    <zone id="edge_cluster_0_zone" routing="Cluster">
      <cluster id="edge_cluster_0" 
               prefix="edge_cluster_0_node_" 
               radical="0-4"
               router_id="edge_cluster_0_router"
               speed="1Gf" core="1" bw="125MBps" lat="50us"
               bb_bw="1GBps" bb_lat="10us"/>
    </zone>
    <!-- Additional clusters... -->
    
    <!-- Inter-cluster links -->
    <link id="link_edge_cluster_0_to_edge_cluster_1" bandwidth="1GBps" latency="5ms"/>
    
    <!-- Inter-cluster routes using zone gateways -->
    <zoneRoute src="edge_cluster_0_zone" dst="edge_cluster_1_zone"
               gw_src="edge_cluster_0_router" gw_dst="edge_cluster_1_router">
      <link_ctn id="link_edge_cluster_0_to_edge_cluster_1"/>
    </zoneRoute>
  </zone>
</platform>
```

### Default Cluster Specifications

**Edge Clusters:**
- Node Speed: 1Gf per core
- Cores: 1 per node
- Internal Bandwidth: 125MBps
- Internal Latency: 50us
- Backbone Bandwidth: 1GBps
- Backbone Latency: 10us

**Fog Clusters:**
- Node Speed: 10Gf per core
- Cores: 4 per node
- Internal Bandwidth: 1GBps
- Internal Latency: 10us
- Backbone Bandwidth: 10GBps
- Backbone Latency: 1us

**Cloud Clusters:**
- Node Speed: 100Gf per core
- Cores: 16 per node
- Internal Bandwidth: 10GBps
- Internal Latency: 1us
- Backbone Bandwidth: 100GBps
- Backbone Latency: 100ns

## Generated Platform Structure

### Host Naming
Clusters generate hosts with consistent naming following SimGrid's radical notation:
```
edge_cluster_0_node_0
edge_cluster_0_node_1
...
edge_cluster_1_node_0
edge_cluster_1_node_1
```

### Router Gateways
Each cluster has a router that acts as a gateway:
```
edge_cluster_0_router
edge_cluster_1_router
```

### Zone Hierarchy
```
edge_platform (Full routing)
  ├── edge_cluster_0_zone (Cluster routing)
  │   └── edge_cluster_0 (cluster tag)
  ├── edge_cluster_1_zone (Cluster routing)
  │   └── edge_cluster_1 (cluster tag)
  └── edge_cluster_2_zone (Cluster routing)
      └── edge_cluster_2 (cluster tag)
```

## Testing

Example test with edge computing app:
```bash
./build/platform_generator edge-cluster 3 5
./build/edge_computing_app ./platforms/edge_platform.xml
```

Expected result:
- Platform loads with 15 hosts (3 clusters × 5 nodes)
- All devices can communicate across clusters via routers

## Comparison: Simple vs Cluster-Based

### Simple (Original) Syntax:
```bash
./build/platform_generator edge 15
```
Generates: 15 individual edge devices + 1 gateway (16 total hosts)
- Each host explicitly defined
- Full mesh routing between all hosts
- Larger XML files

### Cluster Syntax:
```bash
./build/platform_generator edge-cluster 3 5
```
Generates: 3 clusters of 5 nodes each (15 total hosts)
- Compact cluster definitions
- Hierarchical routing (intra-cluster + inter-cluster)
- Smaller, more maintainable XML
