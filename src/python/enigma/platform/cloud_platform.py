"""
CloudPlatform: specialized factories for cloud computing platform topologies.
Mirrors C++ CloudPlatform class from src/platform/CloudPlatform.cpp.
"""

from __future__ import annotations

from .configs import HostConfig, LinkConfig, ZoneConfig


class CloudPlatform:
    """Factory for cloud computing platform topologies."""

    # ------------------------------------------------------------------ #
    # Topology factories                                                   #
    # ------------------------------------------------------------------ #

    @staticmethod
    def create_data_center(
        num_racks: int,
        servers_per_rack: int,
        server_speed: str = "100Gf",
    ) -> ZoneConfig:
        """Fat-tree-style data center: racks of servers connected via spine switch."""
        zone = ZoneConfig("data_center", "Full")
        for r in range(num_racks):
            rack = ZoneConfig(f"rack_{r}", "Full")
            for s in range(servers_per_rack):
                rack.hosts.append(
                    HostConfig(f"server_r{r}_s{s}", server_speed, 32)
                )
            rack.links.append(LinkConfig("rack_switch", "40GBps", "10us"))
            zone.subzones.append(rack)
        zone.links.append(LinkConfig("spine_switch", "10GBps", "50us"))
        return zone

    @staticmethod
    def create_cluster(
        num_servers: int,
        server_speed: str = "100Gf",
        interconnect: str = "10GBps",
    ) -> ZoneConfig:
        """Simple homogeneous cloud cluster."""
        zone = ZoneConfig("cloud_cluster", "Full")
        for i in range(num_servers):
            zone.hosts.append(HostConfig(f"server_{i}", server_speed, 32))
        zone.links.append(LinkConfig("cluster_interconnect", interconnect, "100us"))
        return zone

    @staticmethod
    def create_multi_cloud(
        num_clouds: int,
        servers_per_cloud: int,
        server_speed: str = "100Gf",
    ) -> ZoneConfig:
        """Multiple independent cloud regions connected via WAN links."""
        zone = ZoneConfig("multi_cloud", "Full")
        for c in range(num_clouds):
            cloud = ZoneConfig(f"cloud_{c}", "Full")
            for s in range(servers_per_cloud):
                cloud.hosts.append(
                    HostConfig(f"cloud{c}_server_{s}", server_speed, 32)
                )
            cloud.links.append(LinkConfig("intra_cloud_link", "10GBps", "100us"))
            zone.subzones.append(cloud)
        zone.links.append(LinkConfig("inter_cloud_link", "1GBps", "50ms"))
        return zone

    @staticmethod
    def create_heterogeneous_cluster(
        num_cpu_nodes: int,
        num_gpu_nodes: int,
        cpu_speed: str = "100Gf",
        gpu_speed: str = "500Gf",
    ) -> ZoneConfig:
        """Cluster with both CPU and GPU nodes, connected via InfiniBand."""
        zone = ZoneConfig("heterogeneous_cluster", "Full")
        for i in range(num_cpu_nodes):
            zone.hosts.append(HostConfig(f"cpu_node_{i}", cpu_speed, 64))
        for i in range(num_gpu_nodes):
            zone.hosts.append(HostConfig(f"gpu_node_{i}", gpu_speed, 128))
        zone.links.append(LinkConfig("infiniband", "100GBps", "1us"))
        return zone

    # ------------------------------------------------------------------ #
    # Individual component factories                                       #
    # ------------------------------------------------------------------ #

    @staticmethod
    def create_cloud_server(server_id: str, server_type: str = "standard") -> HostConfig:
        presets = {
            "small":    HostConfig(server_id, "10Gf",  8),
            "standard": HostConfig(server_id, "100Gf", 32),
            "large":    HostConfig(server_id, "500Gf", 64),
            "gpu":      HostConfig(server_id, "1000Gf", 128),
        }
        return presets.get(server_type, HostConfig(server_id, "100Gf", 32))

    @staticmethod
    def create_cloud_link(link_id: str, link_type: str = "10G") -> LinkConfig:
        presets = {
            "1G":   LinkConfig(link_id, "1GBps",   "100us"),
            "10G":  LinkConfig(link_id, "10GBps",  "50us"),
            "40G":  LinkConfig(link_id, "40GBps",  "10us"),
            "100G": LinkConfig(link_id, "100GBps", "1us"),
            "wan":  LinkConfig(link_id, "1GBps",   "50ms"),
        }
        return presets.get(link_type, LinkConfig(link_id, "10GBps", "50us"))
