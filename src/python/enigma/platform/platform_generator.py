"""
PlatformGenerator: Generates SimGrid-compatible platform XML files.

Mirrors the C++ PlatformGenerator class from src/platform/PlatformGenerator.cpp.
Uses Python's built-in xml.dom.minidom for nicely-formatted XML output.
"""

from __future__ import annotations

import os
from typing import List
from xml.dom import minidom
import xml.etree.ElementTree as ET

from .configs import (
    ClusterConfig,
    HostConfig,
    LinkConfig,
    ZoneConfig,
)


class PlatformGenerator:
    """Generates SimGrid 4.1 platform XML files from ZoneConfig trees."""

    # ------------------------------------------------------------------ #
    # Public API                                                           #
    # ------------------------------------------------------------------ #

    def generate_platform(self, filename: str, config: ZoneConfig) -> None:
        """Write a complete SimGrid platform XML file."""
        root_elem = ET.Element("platform", {"version": "4.1"})
        self._write_zone(root_elem, config, is_root=True)

        xml_str = ET.tostring(root_elem, encoding="unicode")
        pretty = self._prettify(xml_str)

        os.makedirs(os.path.dirname(os.path.abspath(filename)), exist_ok=True)
        with open(filename, "w", encoding="utf-8") as f:
            f.write('<?xml version="1.0"?>\n')
            f.write('<!DOCTYPE platform SYSTEM "https://simgrid.org/simgrid.dtd">\n')
            # minidom already prepends <?xml …?> – strip it so we don't duplicate
            lines = pretty.split("\n")
            if lines[0].startswith("<?xml"):
                lines = lines[1:]
            f.write("\n".join(lines))

        print(f"Platform generated: {filename}")

    # ------------------------------------------------------------------ #
    # Static factory helpers (mirror C++ static methods)                  #
    # ------------------------------------------------------------------ #

    @staticmethod
    def create_edge_zone(zone_id: str, num_devices: int) -> ZoneConfig:
        zone = ZoneConfig(zone_id, "Full")
        for i in range(num_devices):
            zone.hosts.append(HostConfig(f"edge_device_{i}", "1Gf", 1))
        zone.links.append(LinkConfig("edge_link", "125MBps", "5ms"))
        return zone

    @staticmethod
    def create_fog_zone(zone_id: str, num_nodes: int) -> ZoneConfig:
        zone = ZoneConfig(zone_id, "Full")
        for i in range(num_nodes):
            zone.hosts.append(HostConfig(f"fog_node_{i}", "10Gf", 4))
        zone.links.append(LinkConfig("fog_link", "1GBps", "2ms"))
        return zone

    @staticmethod
    def create_cloud_zone(zone_id: str, num_servers: int) -> ZoneConfig:
        zone = ZoneConfig(zone_id, "Full")
        for i in range(num_servers):
            zone.hosts.append(HostConfig(f"cloud_server_{i}", "100Gf", 16))
        zone.links.append(LinkConfig("cloud_link", "10GBps", "100us"))
        return zone

    @staticmethod
    def create_hybrid_platform(
        edge_devices: int, fog_nodes: int, cloud_servers: int
    ) -> ZoneConfig:
        edge_clusters = [
            ClusterConfig(f"edge_cluster_{i}", 1, "1Gf", 1, "125MBps", "50us")
            for i in range(edge_devices)
        ]
        fog_clusters = [
            ClusterConfig(f"fog_cluster_{i}", 1, "10Gf", 4, "1GBps", "10us")
            for i in range(fog_nodes)
        ]
        cloud_clusters = [
            ClusterConfig(f"cloud_cluster_{i}", 1, "100Gf", 16, "10GBps", "1us")
            for i in range(cloud_servers)
        ]
        return PlatformGenerator.create_hybrid_with_clusters_flat(
            edge_clusters, fog_clusters, cloud_clusters, edge_cloud_direct=False
        )

    @staticmethod
    def create_edge_with_clusters(zone_id: str, clusters: List[ClusterConfig]) -> ZoneConfig:
        zone = ZoneConfig(zone_id, "Full")
        zone.clusters = list(clusters)
        zone.auto_interconnect = True
        return zone

    @staticmethod
    def create_fog_with_clusters(zone_id: str, clusters: List[ClusterConfig]) -> ZoneConfig:
        zone = ZoneConfig(zone_id, "Full")
        zone.clusters = list(clusters)
        zone.auto_interconnect = True
        return zone

    @staticmethod
    def create_cloud_with_clusters(zone_id: str, clusters: List[ClusterConfig]) -> ZoneConfig:
        zone = ZoneConfig(zone_id, "Full")
        zone.clusters = list(clusters)
        zone.auto_interconnect = True
        return zone

    @staticmethod
    def create_hybrid_with_clusters_flat(
        edge_clusters: List[ClusterConfig],
        fog_clusters: List[ClusterConfig],
        cloud_clusters: List[ClusterConfig],
        edge_cloud_direct: bool = False,
    ) -> ZoneConfig:
        """
        Flat hybrid: all clusters at the same zone level.
        Tiered connectivity: Edge ↔ Fog ↔ Cloud.
        Optionally adds direct Edge ↔ Cloud links.
        """
        root = ZoneConfig("hybrid_platform", "Full")
        root.use_native_clusters = True
        root.force_flat_layout = True
        root.allow_direct_edge_cloud = edge_cloud_direct

        root.clusters.extend(edge_clusters)
        root.clusters.extend(fog_clusters)
        root.clusters.extend(cloud_clusters)

        link_ids: set[str] = set()

        def _add_link(lnk_id: str, bw: str, lat: str) -> None:
            if lnk_id not in link_ids:
                root.links.append(LinkConfig(lnk_id, bw, lat))
                link_ids.add(lnk_id)

        for e in edge_clusters:
            for f in fog_clusters:
                _add_link(f"link_{e.id}_to_{f.id}", "500MBps", "10ms")

        for f in fog_clusters:
            for c in cloud_clusters:
                _add_link(f"link_{f.id}_to_{c.id}", "5GBps", "50ms")

        if edge_cloud_direct:
            for e in edge_clusters:
                for c in cloud_clusters:
                    _add_link(f"link_{e.id}_to_{c.id}", "2GBps", "30ms")

        return root

    # ------------------------------------------------------------------ #
    # Private XML-building helpers                                         #
    # ------------------------------------------------------------------ #

    def _write_zone(
        self, parent: ET.Element, zone: ZoneConfig, is_root: bool = False
    ) -> None:
        zone_elem = ET.SubElement(parent, "zone", {"id": zone.id, "routing": zone.routing})

        # Sub-zones first (hierarchical)
        for subzone in zone.subzones:
            self._write_zone(zone_elem, subzone)

        # Standalone hosts
        for host in zone.hosts:
            self._write_host(zone_elem, host)

        # Clusters (native or expanded)
        if zone.use_native_clusters:
            for cluster in zone.clusters:
                self._write_cluster(zone_elem, cluster)
        else:
            for cluster in zone.clusters:
                self._write_cluster_as_hosts(zone_elem, cluster)

        # Links
        for link in zone.links:
            self._write_link(zone_elem, link)

        # Explicit routes
        for route in zone.explicit_routes:
            self._write_route_config(zone_elem, route)

        # Auto-interconnect logic (mirrors C++ writeZone)
        if zone.auto_interconnect:
            is_flat_hybrid = self._is_flat_hybrid(zone)

            if not is_flat_hybrid and zone.use_native_clusters and len(zone.clusters) > 1:
                # Emit inter-cluster links for single-tier zones
                for i, ci in enumerate(zone.clusters):
                    for cj in zone.clusters[i + 1 :]:
                        link_id = f"link_{ci.id}_to_{cj.id}"
                        existing_ids = {l.id for l in zone.links}
                        if link_id not in existing_ids:
                            self._write_link(zone_elem, LinkConfig(link_id, "1GBps", "5ms"))

            # Routes
            if zone.use_native_clusters and zone.clusters:
                if is_flat_hybrid or zone.force_flat_layout:
                    self._generate_flat_hybrid_routes(zone_elem, zone)
                else:
                    self._generate_cluster_interconnection(zone_elem, zone)
            elif not zone.use_native_clusters and (zone.hosts or zone.clusters):
                self._generate_full_routes_with_clusters(zone_elem, zone)
            elif zone.routing == "Full" and len(zone.hosts) > 1:
                self._generate_full_routes(zone_elem, zone)

            if zone.subzones:
                self._generate_inter_zone_routes(zone_elem, zone)

    # ------------------------------------------------------------------ #

    @staticmethod
    def _is_flat_hybrid(zone: ZoneConfig) -> bool:
        if not zone.use_native_clusters or not zone.clusters:
            return False
        has_edge = any("edge" in c.id for c in zone.clusters)
        has_fog = any("fog" in c.id for c in zone.clusters)
        has_cloud = any("cloud" in c.id for c in zone.clusters)
        return zone.force_flat_layout or (has_edge and has_fog and has_cloud)

    @staticmethod
    def _write_host(parent: ET.Element, host: HostConfig) -> None:
        attrs: dict[str, str] = {"id": host.id, "speed": host.speed}
        if host.core_count > 1:
            attrs["core"] = str(host.core_count)
        if host.coordinates:
            attrs["coordinates"] = host.coordinates
        ET.SubElement(parent, "host", attrs)

    @staticmethod
    def _write_link(parent: ET.Element, link: LinkConfig) -> None:
        attrs: dict[str, str] = {
            "id": link.id,
            "bandwidth": link.bandwidth,
            "latency": link.latency,
        }
        if link.sharing_policy != "SHARED":
            attrs["sharing_policy"] = link.sharing_policy
        ET.SubElement(parent, "link", attrs)

    @staticmethod
    def _write_route(
        parent: ET.Element, src: str, dst: str, link_ids: List[str],
        symmetrical: bool = True,
    ) -> None:
        attrs: dict[str, str] = {"src": src, "dst": dst}
        route_elem = ET.SubElement(parent, "route", attrs)
        for lid in link_ids:
            ET.SubElement(route_elem, "link_ctn", {"id": lid})

    @staticmethod
    def _write_zone_route(
        parent: ET.Element,
        src_zone: str, dst_zone: str,
        gw_src: str, gw_dst: str,
        link_ids: List[str],
    ) -> None:
        attrs: dict[str, str] = {
            "src": src_zone,
            "dst": dst_zone,
            "gw_src": gw_src,
            "gw_dst": gw_dst,
        }
        zr = ET.SubElement(parent, "zoneRoute", attrs)
        for lid in link_ids:
            ET.SubElement(zr, "link_ctn", {"id": lid})

    def _write_route_config(self, parent: ET.Element, rc) -> None:
        if rc.is_zoneRoute:
            self._write_zone_route(
                parent, rc.src, rc.dst, rc.gw_src, rc.gw_dst, rc.link_ids
            )
        else:
            self._write_route(parent, rc.src, rc.dst, rc.link_ids, rc.symmetrical)

    @staticmethod
    def _write_cluster(parent: ET.Element, cluster: ClusterConfig) -> None:
        """Emit a SimGrid <cluster> wrapped in its own <zone routing="Cluster">."""
        cluster_attrs: dict[str, str] = {
            "id": cluster.id,
            "prefix": f"{cluster.id}_node_",
            "suffix": "",
            "radical": f"0-{cluster.num_nodes - 1}",
            "speed": cluster.node_speed,
            "core": str(cluster.cores_per_node),
            "bw": cluster.bandwidth,
            "lat": cluster.latency,
            "bb_bw": cluster.backbone_bw,
            "bb_lat": cluster.backbone_lat,
            "router_id": f"{cluster.id}_router",
        }
        zone_elem = ET.SubElement(
            parent, "zone", {"id": f"{cluster.id}_zone", "routing": "Cluster"}
        )
        ET.SubElement(zone_elem, "cluster", cluster_attrs)

    @staticmethod
    def _write_cluster_as_hosts(parent: ET.Element, cluster: ClusterConfig) -> None:
        """Expand a cluster into individual host + shared link elements."""
        for i in range(cluster.num_nodes):
            host_id = f"{cluster.id}_node_{i}"
            attrs: dict[str, str] = {
                "id": host_id,
                "speed": cluster.node_speed,
            }
            if cluster.cores_per_node > 1:
                attrs["core"] = str(cluster.cores_per_node)
            ET.SubElement(parent, "host", attrs)
        ET.SubElement(
            parent,
            "link",
            {
                "id": f"{cluster.id}_internal_link",
                "bandwidth": cluster.bandwidth,
                "latency": cluster.latency,
            },
        )

    # ------------------------------------------------------------------ #
    # Route-generation (mirrors C++ generateXxx methods)                  #
    # ------------------------------------------------------------------ #

    def _generate_full_routes(self, parent: ET.Element, zone: ZoneConfig) -> None:
        hosts = zone.hosts
        for i, hi in enumerate(hosts):
            for hj in hosts[i + 1 :]:
                # Look for a specific link; fall back to first available
                link_id = None
                for lnk in zone.links:
                    if hi.id in lnk.id and hj.id in lnk.id:
                        link_id = lnk.id
                        break
                if link_id is None and zone.links:
                    link_id = zone.links[0].id
                if link_id:
                    self._write_route(parent, hi.id, hj.id, [link_id])

    def _generate_full_routes_with_clusters(
        self, parent: ET.Element, zone: ZoneConfig
    ) -> None:
        all_hosts: List[str] = [h.id for h in zone.hosts]
        for c in zone.clusters:
            for i in range(c.num_nodes):
                all_hosts.append(f"{c.id}_node_{i}")

        shared_link_id = f"{zone.id}_shared_link"
        if not zone.links:
            ET.SubElement(
                parent,
                "link",
                {"id": shared_link_id, "bandwidth": "1GBps", "latency": "5ms"},
            )
        else:
            shared_link_id = zone.links[0].id

        for i, hi in enumerate(all_hosts):
            for hj in all_hosts[i + 1 :]:
                self._write_route(parent, hi, hj, [shared_link_id])

    def _generate_cluster_interconnection(
        self, parent: ET.Element, zone: ZoneConfig
    ) -> None:
        """Full-mesh zoneRoutes between all clusters in a single-tier zone."""
        for i, ci in enumerate(zone.clusters):
            for cj in zone.clusters[i + 1 :]:
                link_id = f"link_{ci.id}_to_{cj.id}"
                self._write_zone_route(
                    parent,
                    f"{ci.id}_zone",
                    f"{cj.id}_zone",
                    f"{ci.id}_router",
                    f"{cj.id}_router",
                    [link_id],
                )

    def _generate_flat_hybrid_routes(
        self, parent: ET.Element, zone: ZoneConfig
    ) -> None:
        """
        Tiered zoneRoutes for flat hybrid topology:
        Edge → Fog and Fog → Cloud.
        Optionally Edge → Cloud when allow_direct_edge_cloud is set.
        """
        edge_c = [c for c in zone.clusters if "edge" in c.id]
        fog_c = [c for c in zone.clusters if "fog" in c.id]
        cloud_c = [c for c in zone.clusters if "cloud" in c.id]

        for e in edge_c:
            for f in fog_c:
                self._write_zone_route(
                    parent,
                    f"{e.id}_zone", f"{f.id}_zone",
                    f"{e.id}_router", f"{f.id}_router",
                    [f"link_{e.id}_to_{f.id}"],
                )

        for f in fog_c:
            for c in cloud_c:
                self._write_zone_route(
                    parent,
                    f"{f.id}_zone", f"{c.id}_zone",
                    f"{f.id}_router", f"{c.id}_router",
                    [f"link_{f.id}_to_{c.id}"],
                )

        if zone.allow_direct_edge_cloud:
            for e in edge_c:
                for c in cloud_c:
                    self._write_zone_route(
                        parent,
                        f"{e.id}_zone", f"{c.id}_zone",
                        f"{e.id}_router", f"{c.id}_router",
                        [f"link_{e.id}_to_{c.id}"],
                    )

    def _generate_inter_zone_routes(
        self, parent: ET.Element, zone: ZoneConfig
    ) -> None:
        """Generate zoneRoutes between sub-zones that have clusters."""
        subzones = zone.subzones
        for i, zi in enumerate(subzones):
            for zj in subzones[i + 1 :]:
                if not zi.clusters or not zj.clusters:
                    continue
                # Skip direct edge→cloud (should go through fog)
                if "edge" in zi.id and "cloud" in zj.id:
                    continue
                if "edge" in zj.id and "cloud" in zi.id:
                    continue

                gw_i = f"{zi.clusters[0].id}_router"
                gw_j = f"{zj.clusters[0].id}_router"

                link_id: str | None = None
                if "edge" in zi.id and "fog" in zj.id:
                    link_id = "edge_to_fog"
                elif "fog" in zi.id and "cloud" in zj.id:
                    link_id = "fog_to_cloud"
                elif zone.links:
                    link_id = zone.links[0].id

                if link_id:
                    self._write_zone_route(
                        parent, zi.id, zj.id, gw_i, gw_j, [link_id]
                    )

    # ------------------------------------------------------------------ #
    # Formatting                                                           #
    # ------------------------------------------------------------------ #

    @staticmethod
    def _prettify(xml_str: str) -> str:
        """Return a pretty-printed XML string with 2-space indentation."""
        dom = minidom.parseString(xml_str)
        return dom.toprettyxml(indent="  ")
