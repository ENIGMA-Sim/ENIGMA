"""
MQTTConfig: configures whether MQTT is active and which broker to use.
"""
from __future__ import annotations

from dataclasses import dataclass, field


@dataclass
class MQTTConfig:
    """MQTT configuration options."""
    enabled: bool = False
    broker_name: str = "mqtt_broker"
    broker_host: str = ""
    auto_start_broker: bool = True
    default_qos: int = 0

    @classmethod
    def create_disabled(cls) -> "MQTTConfig":
        return cls(enabled=False)

    @classmethod
    def create_enabled(cls, broker_host: str = "") -> "MQTTConfig":
        return cls(enabled=True, broker_host=broker_host)
