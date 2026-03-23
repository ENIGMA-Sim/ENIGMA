"""
Data classes that mirror C++ structs from include/platform/PlatformGenerator.hpp.
Represent the configuration objects used to build SimGrid platform XML files.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum, auto
from typing import List


class InfrastructureType(Enum):
    """Type of computing infrastructure tier."""
    EDGE = auto()
    FOG = auto()
    CLOUD = auto()
    HYBRID = auto()


@dataclass
class HostConfig:
    """Configuration for a single SimGrid host node."""
    id: str
    speed: str              # CPU speed, e.g. "1Gf", "10Gf", "100Gf"
    core_count: int = 1
    coordinates: str = ""   # Optional vivaldi coordinates

    def __post_init__(self) -> None:
        if not self.id:
            raise ValueError("HostConfig.id must not be empty")
        if not self.speed:
            raise ValueError("HostConfig.speed must not be empty")


@dataclass
class LinkConfig:
    """Configuration for a SimGrid network link."""
    id: str
    bandwidth: str          # e.g. "125MBps", "1GBps", "10GBps"
    latency: str = "50us"   # e.g. "50us", "10ms"
    sharing_policy: str = "SHARED"   # "SHARED" or "FATPIPE"


@dataclass
class ClusterConfig:
    """Configuration for a SimGrid cluster (maps to <cluster> tag)."""
    id: str
    num_nodes: int
    node_speed: str         # CPU speed per node, e.g. "1Gf"
    cores_per_node: int = 1
    bandwidth: str = "125MBps"   # Internal cluster bandwidth
    latency: str = "50us"        # Internal cluster latency
    backbone_bw: str = "1GBps"   # Backbone/uplink bandwidth
    backbone_lat: str = "10us"   # Backbone/uplink latency


@dataclass
class RouteConfig:
    """Configuration for a route between two zones/hosts."""
    src: str
    dst: str
    link_ids: List[str] = field(default_factory=list)
    symmetrical: bool = True
    is_zoneRoute: bool = False  # True → <zoneRoute>, False → <route>
    gw_src: str = ""            # Required for zoneRoute
    gw_dst: str = ""            # Required for zoneRoute


@dataclass
class ZoneConfig:
    """
    Configuration for a SimGrid network zone.

    Mirrors C++ struct ZoneConfig from include/platform/PlatformGenerator.hpp.
    A ZoneConfig can contain hosts, links, clusters, sub-zones and explicit routes.
    """
    id: str
    routing: str = "Full"       # "Full", "Floyd", "Dijkstra", "Cluster", "None"
    hosts: List[HostConfig] = field(default_factory=list)
    links: List[LinkConfig] = field(default_factory=list)
    clusters: List[ClusterConfig] = field(default_factory=list)
    subzones: List["ZoneConfig"] = field(default_factory=list)
    explicit_routes: List[RouteConfig] = field(default_factory=list)

    # Generation behaviour flags
    auto_interconnect: bool = True
    use_native_clusters: bool = True
    allow_direct_edge_cloud: bool = False
    force_flat_layout: bool = False
