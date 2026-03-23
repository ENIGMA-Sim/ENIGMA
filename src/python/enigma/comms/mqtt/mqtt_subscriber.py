"""
MQTTSubscriber: subscribes to topics and receives messages via the simulated broker.
"""
from __future__ import annotations

from typing import List, Optional
import simgrid
from .mqtt_broker import MQTTBroker, MQTTMessage, _ControlMessage, _ControlType


class MQTTSubscriber:
    """
    A subscriber registers itself with the broker over the control mailbox
    and subsequently receives messages from the broker via a per-subscriber
    delivery mailbox.

    Parameters
    ----------
    broker_name:
        Must match the name used when calling :func:`~.mqtt_broker.start_broker`.
    subscriber_id:
        Optional human-readable name; defaults to the current actor's host name
        combined with a random suffix to guarantee uniqueness.
    """

    def __init__(
        self,
        broker_name: str = "mqtt_broker",
        subscriber_id: str = "",
    ) -> None:
        self.broker_name = broker_name
        host_name = simgrid.this_actor.get_host().name
        self._subscriber_id = subscriber_id or f"{host_name}_sub_{id(self)}"
        self._broker_mbox = simgrid.Mailbox.by_name(
            MQTTBroker.get_broker_mailbox(broker_name)
        )
        # Unique delivery mailbox for this subscriber instance
        self._my_mbox = simgrid.Mailbox.by_name(self._delivery_mbox_name)
        self._subscribed_topics: List[str] = []

    # ------------------------------------------------------------------ #
    # Subscription management                                             #
    # ------------------------------------------------------------------ #

    def subscribe(self, topic: str) -> None:
        """Subscribe to *topic*; messages will be routed to this subscriber's mailbox."""
        if topic not in self._subscribed_topics:
            # Tell broker about the subscription using the *delivery* mailbox name
            ctrl = _ControlMessage(
                ctrl_type=_ControlType.SUBSCRIBE,
                topic=topic,
                subscriber=self._delivery_mbox_name,
            )
            self._broker_mbox.put(ctrl, 64)   # small control packet
            self._subscribed_topics.append(topic)

    def unsubscribe(self, topic: str) -> None:
        """Unsubscribe from *topic*."""
        if topic in self._subscribed_topics:
            ctrl = _ControlMessage(
                ctrl_type=_ControlType.UNSUBSCRIBE,
                topic=topic,
                subscriber=self._delivery_mbox_name,
            )
            self._broker_mbox.put(ctrl, 64)
            self._subscribed_topics.remove(topic)

    # ------------------------------------------------------------------ #
    # Receiving messages                                                  #
    # ------------------------------------------------------------------ #

    def receive(self, timeout: float = -1.0) -> Optional[MQTTMessage]:
        """
        Block until a message arrives or until *timeout* seconds elapse.

        Parameters
        ----------
        timeout:
            Maximum wait in simulated seconds.  Use ``-1`` for indefinite wait.

        Returns
        -------
        :class:`~.mqtt_broker.MQTTMessage` or ``None`` if timeout was reached.
        """
        try:
            if timeout < 0:
                return self._my_mbox.get()
            else:
                comm = self._my_mbox.get_async()
                comm.wait_for(timeout)
                return comm.get_payload()
        except (simgrid.TimeoutException, simgrid.NetworkFailureException):
            return None
        except Exception:
            return None

    def has_messages(self) -> bool:
        """Return True if there is at least one message waiting (non-blocking check)."""
        try:
            return self._my_mbox.ready
        except Exception:
            return False

    # ------------------------------------------------------------------ #
    # Properties                                                          #
    # ------------------------------------------------------------------ #

    @property
    def subscriber_id(self) -> str:
        return self._subscriber_id

    @property
    def mailbox_name(self) -> str:
        return self._delivery_mbox_name

    @property
    def subscribed_topics(self) -> List[str]:
        return list(self._subscribed_topics)

    @property
    def _delivery_mbox_name(self) -> str:
        return f"subscriber_{self._subscriber_id}"
