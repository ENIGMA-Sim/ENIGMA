"""
FogPlatform: specialized factories for fog computing platform topologies.
Mirrors C++ FogPlatform class from src/platform/FogPlatform.cpp.
"""

from __future__ import annotations

from .configs import HostConfig, LinkConfig, ZoneConfig


class FogPlatform:
    """Factory for fog computing platform topologies."""

    # ------------------------------------------------------------------ #
    # Topology factories                                                   #
    # ------------------------------------------------------------------ #

    @staticmethod
    def create_hierarchical_topology(
        num_fog_nodes: int,
        node_speed: str = "10Gf",
    ) -> ZoneConfig:
        """Flat hierarchy of interconnected fog nodes."""
        zone = ZoneConfig("fog_hierarchical", "Full")
        for i in range(num_fog_nodes):
            zone.hosts.append(HostConfig(f"fog_node_{i}", node_speed, 8))
        for i in range(num_fog_nodes):
            for j in range(i + 1, num_fog_nodes):
                zone.links.append(LinkConfig(f"fog_link_{i}_{j}", "1GBps", "5ms"))
        return zone

    @staticmethod
    def create_edge_fog_topology(
        num_fog_nodes: int,
        edge_devices_per_node: int,
        fog_speed: str = "10Gf",
        edge_speed: str = "1Gf",
    ) -> ZoneConfig:
        """
        Hierarchical edge-fog topology:
        each fog node owns a sub-zone of edge devices.
        """
        zone = ZoneConfig("edge_fog_topology", "Full")
        for i in range(num_fog_nodes):
            zone.hosts.append(HostConfig(f"fog_node_{i}", fog_speed, 8))
            sub = ZoneConfig(f"edge_zone_{i}", "Full")
            for j in range(edge_devices_per_node):
                sub.hosts.append(HostConfig(f"edge_{i}_{j}", edge_speed, 1))
            sub.links.append(LinkConfig("edge_to_fog_link", "500MBps", "8ms"))
            zone.subzones.append(sub)
        zone.links.append(LinkConfig("fog_interconnect", "1GBps", "3ms"))
        return zone

    @staticmethod
    def create_geographic_topology(
        num_regions: int,
        nodes_per_region: int,
        node_speed: str = "10Gf",
    ) -> ZoneConfig:
        """Geographic regions, each with their own set of fog nodes."""
        zone = ZoneConfig("fog_geographic", "Full")
        for r in range(num_regions):
            region = ZoneConfig(f"region_{r}", "Full")
            for n in range(nodes_per_region):
                region.hosts.append(HostConfig(f"fog_r{r}_n{n}", node_speed, 8))
            region.links.append(LinkConfig("intra_region_link", "1GBps", "2ms"))
            zone.subzones.append(region)
        zone.links.append(LinkConfig("inter_region_link", "500MBps", "20ms"))
        return zone

    # ------------------------------------------------------------------ #
    # Individual component factories                                       #
    # ------------------------------------------------------------------ #

    @staticmethod
    def create_fog_node(node_id: str, node_type: str = "standard") -> HostConfig:
        presets = {
            "lightweight": HostConfig(node_id, "5Gf",  4),
            "standard":    HostConfig(node_id, "10Gf", 8),
            "powerful":    HostConfig(node_id, "20Gf", 16),
            "edge_server": HostConfig(node_id, "15Gf", 12),
        }
        return presets.get(node_type, HostConfig(node_id, "10Gf", 8))

    @staticmethod
    def create_fog_link(link_id: str, link_type: str = "ethernet") -> LinkConfig:
        presets = {
            "ethernet":  LinkConfig(link_id, "1GBps",  "2ms"),
            "fiber":     LinkConfig(link_id, "10GBps", "500us"),
            "wireless":  LinkConfig(link_id, "300MBps","5ms"),
            "wan":       LinkConfig(link_id, "100MBps","20ms"),
        }
        return presets.get(link_type, LinkConfig(link_id, "1GBps", "2ms"))
