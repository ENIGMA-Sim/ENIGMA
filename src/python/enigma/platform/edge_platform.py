"""
EdgePlatform: specialized factories for edge/IoT platform topologies.
Mirrors C++ EdgePlatform class from src/platform/EdgePlatform.cpp.
"""

from __future__ import annotations

from .configs import HostConfig, LinkConfig, ZoneConfig


class EdgePlatform:
    """Factory for edge computing platform topologies."""

    # ------------------------------------------------------------------ #
    # Topology factories                                                   #
    # ------------------------------------------------------------------ #

    @staticmethod
    def create_star_topology(
        num_devices: int,
        device_speed: str = "1Gf",
        gateway_speed: str = "5Gf",
    ) -> ZoneConfig:
        """Star topology: one central gateway + N edge devices."""
        zone = ZoneConfig("edge_star", "Full")
        zone.hosts.append(HostConfig("edge_gateway", gateway_speed, 2))
        for i in range(num_devices):
            zone.hosts.append(HostConfig(f"edge_device_{i}", device_speed, 1))
            zone.links.append(LinkConfig(f"link_device_{i}_gateway", "100MBps", "10ms"))
        return zone

    @staticmethod
    def create_mesh_topology(
        num_devices: int,
        device_speed: str = "1Gf",
    ) -> ZoneConfig:
        """Full-mesh topology: every device connected to every other device."""
        zone = ZoneConfig("edge_mesh", "Full")
        for i in range(num_devices):
            zone.hosts.append(HostConfig(f"edge_device_{i}", device_speed, 1))
        for i in range(num_devices):
            for j in range(i + 1, num_devices):
                zone.links.append(LinkConfig(f"link_{i}_{j}", "50MBps", "15ms"))
        return zone

    @staticmethod
    def create_iot_platform(
        num_sensors: int,
        num_actuators: int,
        gateway_speed: str = "3Gf",
    ) -> ZoneConfig:
        """
        IoT platform: one gateway + low-power sensors + actuators,
        each connected via a dedicated wireless link.
        """
        zone = ZoneConfig("iot_platform", "Full")
        zone.hosts.append(HostConfig("iot_gateway", gateway_speed, 2))
        for i in range(num_sensors):
            zone.hosts.append(HostConfig(f"sensor_{i}", "500Mf", 1))
            zone.links.append(LinkConfig(f"link_sensor_{i}", "10MBps", "20ms"))
        for i in range(num_actuators):
            zone.hosts.append(HostConfig(f"actuator_{i}", "800Mf", 1))
            zone.links.append(LinkConfig(f"link_actuator_{i}", "20MBps", "15ms"))
        return zone

    # ------------------------------------------------------------------ #
    # Individual component factories                                       #
    # ------------------------------------------------------------------ #

    @staticmethod
    def create_edge_device(
        device_id: str, device_type: str = "standard"
    ) -> HostConfig:
        """Return a HostConfig for a typical edge device type."""
        presets = {
            "sensor":       HostConfig(device_id, "500Mf", 1),
            "smartphone":   HostConfig(device_id, "2Gf", 4),
            "raspberry_pi": HostConfig(device_id, "1.5Gf", 4),
            "gateway":      HostConfig(device_id, "5Gf", 2),
        }
        return presets.get(device_type, HostConfig(device_id, "1Gf", 1))

    @staticmethod
    def create_edge_link(link_id: str, link_type: str = "wifi") -> LinkConfig:
        """Return a LinkConfig for a typical edge link type."""
        presets = {
            "wifi":      LinkConfig(link_id, "54MBps",   "10ms"),
            "5g":        LinkConfig(link_id, "1GBps",    "5ms"),
            "zigbee":    LinkConfig(link_id, "250KBps",  "20ms"),
            "bluetooth": LinkConfig(link_id, "3MBps",    "8ms"),
            "ethernet":  LinkConfig(link_id, "1GBps",    "1ms"),
            "lora":      LinkConfig(link_id, "50KBps",   "100ms"),
        }
        return presets.get(link_type, LinkConfig(link_id, "100MBps", "10ms"))
