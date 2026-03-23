"""
MQTTPublisher: publishes messages to topics via the simulated MQTT broker.
"""
from __future__ import annotations

import simgrid
from .mqtt_broker import MQTTBroker, MQTTMessage, _ControlMessage, _ControlType


class MQTTPublisher:
    """
    Published messages are sent as a :class:`~.mqtt_broker._ControlMessage`
    (type=PUBLISH) to the broker's control mailbox.  The broker then fans
    the message out to all registered subscribers.

    Parameters
    ----------
    broker_name:
        Must match the name used when calling :func:`~.mqtt_broker.start_broker`.
    publisher_id:
        Optional human-readable name; defaults to the current actor's host name.
    """

    def __init__(
        self,
        broker_name: str = "mqtt_broker",
        publisher_id: str = "",
    ) -> None:
        self.broker_name = broker_name
        self._publisher_id = publisher_id or simgrid.this_actor.get_host().name
        self._broker_mbox = simgrid.Mailbox.by_name(
            MQTTBroker.get_broker_mailbox(broker_name)
        )

    # ------------------------------------------------------------------ #
    # Public interface                                                     #
    # ------------------------------------------------------------------ #

    def publish(
        self,
        topic: str,
        payload: str,
        size: int = 0,
        qos: int = 0,
    ) -> None:
        """
        Publish *payload* to *topic*.

        Parameters
        ----------
        topic:
            MQTT topic string, e.g. ``"sensors/temperature"``.
        payload:
            Message body string.
        size:
            Byte size used for SimGrid network simulation.
            Defaults to ``len(payload)`` if 0.
        qos:
            MQTT Quality of Service level (0, 1, or 2).
        """
        byte_size = size if size > 0 else len(payload.encode())
        msg = MQTTMessage(
            topic=topic,
            payload=payload,
            size=byte_size,
            publisher=self._publisher_id,
            qos=qos,
        )
        ctrl = _ControlMessage(
            ctrl_type=_ControlType.PUBLISH,
            topic=topic,
            message=msg,
        )
        # The network transfer cost is counted by the broker mailbox put
        self._broker_mbox.put(ctrl, byte_size)

    @property
    def publisher_id(self) -> str:
        return self._publisher_id
