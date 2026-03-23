"""
MQTTBroker: SimGrid-based MQTT broker simulation.

Runs as a SimGrid actor on a designated host.
Uses SimGrid Mailboxes to simulate the pub/sub message routing.
"""
from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Optional
import simgrid


# --------------------------------------------------------------------------- #
# Data structures                                                              #
# --------------------------------------------------------------------------- #

@dataclass
class MQTTMessage:
    """In-simulation MQTT message."""
    topic: str
    payload: str
    size: int                       # bytes
    publisher: str
    qos: int = 0
    timestamp: float = 0.0

    def __post_init__(self) -> None:
        if self.timestamp == 0.0:
            try:
                self.timestamp = simgrid.Engine.clock
            except Exception:
                pass


class _ControlType:
    SUBSCRIBE   = "SUBSCRIBE"
    UNSUBSCRIBE = "UNSUBSCRIBE"
    PUBLISH     = "PUBLISH"
    SHUTDOWN    = "SHUTDOWN"


@dataclass
class _ControlMessage:
    ctrl_type: str
    topic: str = ""
    subscriber: str = ""
    message: Optional[MQTTMessage] = None


# --------------------------------------------------------------------------- #
# MQTTBroker actor                                                             #
# --------------------------------------------------------------------------- #

class MQTTBroker:
    """
    Central MQTT broker.  Run as a SimGrid actor via :func:`start_broker`.

    The broker listens on a well-known mailbox
    ``<broker_name>_ctrl`` for :class:`_ControlMessage` objects and
    routes :class:`MQTTMessage` objects to per-topic mailboxes whose name
    is ``<broker_name>_topic_<topic>``.
    """

    def __init__(self, name: str = "mqtt_broker") -> None:
        self.broker_name = name
        self._subscriptions: Dict[str, List[str]] = {}
        self._messages_published = 0
        self._messages_delivered = 0
        self._running = True

    # -- SimGrid actor entry point ------------------------------------------ #

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(
            f"MQTT Broker '{self.broker_name}' started on host '{host.name}'"
        )
        ctrl_mbox = simgrid.Mailbox.by_name(self._ctrl_mbox_name)

        while self._running:
            try:
                comm = ctrl_mbox.get_async()
                comm.wait_for(10.0)          # 10 s idle → exit
                ctrl = comm.get_payload()
            except (simgrid.TimeoutException, simgrid.NetworkFailureException):
                simgrid.this_actor.info("Broker idle timeout – shutting down")
                break
            if not isinstance(ctrl, _ControlMessage):
                simgrid.this_actor.warning(
                    f"Broker received unexpected object type: {type(ctrl)}"
                )
                continue

            if ctrl.ctrl_type == _ControlType.SUBSCRIBE:
                self._handle_subscribe(ctrl.topic, ctrl.subscriber)
            elif ctrl.ctrl_type == _ControlType.UNSUBSCRIBE:
                self._handle_unsubscribe(ctrl.topic, ctrl.subscriber)
            elif ctrl.ctrl_type == _ControlType.PUBLISH:
                self._handle_publish(ctrl.message)
            elif ctrl.ctrl_type == _ControlType.SHUTDOWN:
                simgrid.this_actor.info("Broker shutdown requested")
                self._running = False

        self._print_stats()
        simgrid.this_actor.info(f"MQTT Broker '{self.broker_name}' terminated")

    # -- Internal helpers --------------------------------------------------- #

    def _handle_subscribe(self, topic: str, subscriber: str) -> None:
        subs = self._subscriptions.setdefault(topic, [])
        if subscriber not in subs:
            subs.append(subscriber)
            simgrid.this_actor.info(
                f"Subscriber '{subscriber}' subscribed to topic '{topic}' "
                f"({len(subs)} subscribers)"
            )

    def _handle_unsubscribe(self, topic: str, subscriber: str) -> None:
        if topic in self._subscriptions:
            try:
                self._subscriptions[topic].remove(subscriber)
            except ValueError:
                pass
            if not self._subscriptions[topic]:
                del self._subscriptions[topic]

    def _handle_publish(self, msg: MQTTMessage) -> None:
        self._messages_published += 1
        simgrid.this_actor.info(
            f"Publishing to topic '{msg.topic}' ({msg.size} bytes, from '{msg.publisher}')"
        )
        subs = self._subscriptions.get(msg.topic, [])
        if not subs:
            simgrid.this_actor.info(f"No subscribers for topic '{msg.topic}'")
            return
        for sub_mbox_name in subs:
            dest = simgrid.Mailbox.by_name(sub_mbox_name)
            dest.put(msg, msg.size)
            self._messages_delivered += 1

    def _print_stats(self) -> None:
        simgrid.this_actor.info(
            f"Broker stats – published: {self._messages_published}, "
            f"delivered: {self._messages_delivered}"
        )

    # -- Static helpers ------------------------------------------------------ #

    @property
    def _ctrl_mbox_name(self) -> str:
        return MQTTBroker.get_broker_mailbox(self.broker_name)

    @staticmethod
    def get_broker_mailbox(broker_name: str = "mqtt_broker") -> str:
        """Return the control mailbox name for a given broker."""
        return f"{broker_name}_ctrl"

    @staticmethod
    def get_topic_mailbox(broker_name: str, topic: str) -> str:
        """Return the delivery mailbox name for a given broker+topic pair."""
        safe_topic = topic.replace("/", "_").replace("#", "all").replace("+", "any")
        return f"{broker_name}_topic_{safe_topic}"


# --------------------------------------------------------------------------- #
# Convenience launcher                                                         #
# --------------------------------------------------------------------------- #

def start_broker(host: "simgrid.Host", broker_name: str = "mqtt_broker") -> None:
    """
    Convenience function: launch an :class:`MQTTBroker` actor on *host*.

    Call this once before any publishers or subscribers are created::

        start_broker(broker_host, "mqtt_broker")
    """
    simgrid.Actor.create(f"mqtt_broker_{broker_name}", host, MQTTBroker(broker_name))
