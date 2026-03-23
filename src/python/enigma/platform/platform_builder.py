"""
PlatformBuilder: fluent (builder-pattern) API for constructing platforms.
Mirrors C++ PlatformBuilder class from include/platform/PlatformBuilder.hpp.
"""

from __future__ import annotations

import io
from typing import List, Optional

from .configs import ClusterConfig, HostConfig, LinkConfig, RouteConfig, ZoneConfig
from .platform_generator import PlatformGenerator


class PlatformBuilder:
    """
    Fluent builder for SimGrid platform configurations.

    Usage example::

        xml_str = (
            PlatformBuilder()
            .create_platform("my_platform")
            .add_edge_layer(num_devices=4)
            .add_fog_layer(num_nodes=2)
            .add_cloud_layer(num_servers=1)
            .get_platform_xml()
        )
    """

    def __init__(self) -> None:
        self._platform_name: str = "enigma_platform"
        self._root_zone: ZoneConfig = ZoneConfig("root", "Full")
        self._current_zone: ZoneConfig = self._root_zone
        self._default_latency: str = "50us"
        self._generator: PlatformGenerator = PlatformGenerator()

    # ------------------------------------------------------------------ #
    # Initialisation                                                       #
    # ------------------------------------------------------------------ #

    def create_platform(self, name: str) -> "PlatformBuilder":
        self._platform_name = name
        self._root_zone = ZoneConfig(name, "Full")
        self._current_zone = self._root_zone
        return self

    def create_edge_fog_cloud(self, name: str) -> "PlatformBuilder":
        return self.create_platform(name)

    # ------------------------------------------------------------------ #
    # Layer factories                                                      #
    # ------------------------------------------------------------------ #

    def add_edge_layer(
        self,
        num_devices: int,
        speed: str = "1Gf",
        bandwidth: str = "125MBps",
    ) -> "PlatformBuilder":
        """Add an edge tier with *num_devices* devices as a cluster."""
        from .platform_generator import PlatformGenerator as PG
        clusters = [
            ClusterConfig(
                f"edge_cluster_{i}", 1, speed, 1, bandwidth, self._default_latency
            )
            for i in range(num_devices)
        ]
        self._root_zone.clusters.extend(clusters)
        self._root_zone.force_flat_layout = True
        return self

    def add_fog_layer(
        self,
        num_nodes: int,
        speed: str = "10Gf",
        bandwidth: str = "1GBps",
    ) -> "PlatformBuilder":
        """Add a fog tier with *num_nodes* nodes as a cluster."""
        clusters = [
            ClusterConfig(
                f"fog_cluster_{i}", 1, speed, 4, bandwidth, self._default_latency
            )
            for i in range(num_nodes)
        ]
        self._root_zone.clusters.extend(clusters)
        self._root_zone.force_flat_layout = True
        return self

    def add_cloud_layer(
        self,
        num_servers: int,
        speed: str = "100Gf",
        bandwidth: str = "10GBps",
    ) -> "PlatformBuilder":
        """Add a cloud tier with *num_servers* servers as a cluster."""
        clusters = [
            ClusterConfig(
                f"cloud_cluster_{i}", 1, speed, 16, bandwidth, self._default_latency
            )
            for i in range(num_servers)
        ]
        self._root_zone.clusters.extend(clusters)
        self._root_zone.force_flat_layout = True
        return self

    # ------------------------------------------------------------------ #
    # Custom element addition                                              #
    # ------------------------------------------------------------------ #

    def add_zone(
        self, zone_id: str, routing: str = "Full"
    ) -> "PlatformBuilder":
        sub = ZoneConfig(zone_id, routing)
        self._current_zone.subzones.append(sub)
        self._current_zone = sub
        return self

    def add_host(
        self, host_id: str, speed: str, cores: int = 1
    ) -> "PlatformBuilder":
        self._current_zone.hosts.append(HostConfig(host_id, speed, cores))
        return self

    def add_link(
        self,
        link_id: str,
        bandwidth: str,
        latency: Optional[str] = None,
    ) -> "PlatformBuilder":
        lat = latency or self._default_latency
        self._current_zone.links.append(LinkConfig(link_id, bandwidth, lat))
        return self

    def add_route(
        self, src: str, dst: str, link_ids: List[str]
    ) -> "PlatformBuilder":
        self._current_zone.explicit_routes.append(
            RouteConfig(src=src, dst=dst, link_ids=link_ids)
        )
        return self

    def add_cluster(self, cluster: ClusterConfig) -> "PlatformBuilder":
        self._current_zone.clusters.append(cluster)
        return self

    # ------------------------------------------------------------------ #
    # Advanced configuration                                               #
    # ------------------------------------------------------------------ #

    def set_routing(self, routing: str) -> "PlatformBuilder":
        self._current_zone.routing = routing
        return self

    def set_latency(self, default_latency: str) -> "PlatformBuilder":
        self._default_latency = default_latency
        return self

    def set_direct_edge_cloud(self, enabled: bool = True) -> "PlatformBuilder":
        self._root_zone.allow_direct_edge_cloud = enabled
        return self

    # ------------------------------------------------------------------ #
    # Build / output                                                       #
    # ------------------------------------------------------------------ #

    def build(self) -> None:
        """Write the platform to *<platform_name>.xml* in the platforms/ directory."""
        self.build_to_file(f"platforms/{self._platform_name}.xml")

    def build_to_file(self, filename: str) -> None:
        """Write the platform XML to *filename*."""
        self._validate()
        self._add_inter_tier_links()
        self._generator.generate_platform(filename, self._root_zone)

    def get_platform_xml(self) -> str:
        """Return the platform XML as a string (does not write a file)."""
        import tempfile, os
        with tempfile.NamedTemporaryFile(
            suffix=".xml", delete=False, mode="w"
        ) as tmp:
            tmp_path = tmp.name
        try:
            self.build_to_file(tmp_path)
            with open(tmp_path, encoding="utf-8") as f:
                return f.read()
        finally:
            if os.path.exists(tmp_path):
                os.unlink(tmp_path)

    # ------------------------------------------------------------------ #
    # Internals                                                            #
    # ------------------------------------------------------------------ #

    def _validate(self) -> None:
        if not self._root_zone.hosts and not self._root_zone.clusters and not self._root_zone.subzones:
            raise ValueError("Platform has no hosts, clusters or sub-zones defined.")

    def _add_inter_tier_links(self) -> None:
        """
        Auto-generate tiered inter-cluster links if the builder was used
        to add edge/fog/cloud layers without explicit link creation.
        """
        existing_link_ids = {lnk.id for lnk in self._root_zone.links}

        edge_c = [c for c in self._root_zone.clusters if "edge" in c.id]
        fog_c  = [c for c in self._root_zone.clusters if "fog" in c.id]
        cloud_c = [c for c in self._root_zone.clusters if "cloud" in c.id]

        for e in edge_c:
            for f in fog_c:
                lid = f"link_{e.id}_to_{f.id}"
                if lid not in existing_link_ids:
                    self._root_zone.links.append(LinkConfig(lid, "500MBps", "10ms"))
                    existing_link_ids.add(lid)

        for f in fog_c:
            for c in cloud_c:
                lid = f"link_{f.id}_to_{c.id}"
                if lid not in existing_link_ids:
                    self._root_zone.links.append(LinkConfig(lid, "5GBps", "50ms"))
                    existing_link_ids.add(lid)

        if self._root_zone.allow_direct_edge_cloud:
            for e in edge_c:
                for c in cloud_c:
                    lid = f"link_{e.id}_to_{c.id}"
                    if lid not in existing_link_ids:
                        self._root_zone.links.append(LinkConfig(lid, "2GBps", "30ms"))
                        existing_link_ids.add(lid)
