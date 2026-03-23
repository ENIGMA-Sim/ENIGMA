"""
edge_computing.py – Simulates edge devices that process data locally and
forward results to a central gateway.

Mirrors tests/edge_computing.cpp.

Usage
-----
    python edge_computing.py <platform_file.xml>
"""
from __future__ import annotations

import sys
import os

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.join(_HERE, "..")
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

import simgrid


class EdgeDevice:
    """Senses data, processes locally, sends results to a gateway mailbox."""

    def __init__(
        self,
        gateway_name: str,
        computation_size: float = 2e8,
        data_size: float = 1e5,
    ) -> None:
        self._gateway      = gateway_name
        self._computation  = computation_size   # Flops
        self._data_size    = data_size           # bytes

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(
            f"Edge Device '{host.name}' started "
            f"(speed: {host.speed / 1e9:.2f} Gf)"
        )

        # Phase 1: data collection
        simgrid.this_actor.info("Collecting sensor data…")
        simgrid.this_actor.sleep_for(1.0)

        # Phase 2: local processing
        simgrid.this_actor.info(
            f"Processing data locally ({self._computation / 1e6:.2f} MFlops)…"
        )
        simgrid.this_actor.execute(self._computation)

        # Phase 3: send results to gateway
        simgrid.this_actor.info(f"Sending results to gateway ({self._gateway})…")
        mbox = simgrid.Mailbox.by_name(self._gateway)
        mbox.put(f"Processed data from {host.name}", int(self._data_size))

        simgrid.this_actor.info("Task completed")


class EdgeGateway:
    """Collects results from all edge devices and performs aggregation."""

    def __init__(self, num_devices: int) -> None:
        self._num_devices = num_devices

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(f"Edge Gateway '{host.name}' started")

        mbox = simgrid.Mailbox.by_name(host.name)
        for i in range(self._num_devices):
            msg = mbox.get()
            simgrid.this_actor.info(f"Received: {msg}")
            simgrid.this_actor.execute(5e8)   # 500 MFlops per aggregation

        simgrid.this_actor.info("All data processed. Gateway completed.")


def main() -> None:
    e = simgrid.Engine(sys.argv)

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <platform_file.xml>", file=sys.stderr)
        sys.exit(1)

    e.load_platform(sys.argv[1])
    hosts = e.all_hosts

    if not hosts:
        print("No hosts found in platform", file=sys.stderr)
        sys.exit(1)

    simgrid.this_actor.info("=== Edge Computing Application ===")
    simgrid.this_actor.info(f"Platform loaded with {len(hosts)} hosts")

    # Identify gateway by name; fall back to last host if not found
    gateway_host = next((h for h in hosts if "gateway" in h.name), hosts[-1])
    device_hosts = [h for h in hosts if h is not gateway_host]

    if not device_hosts:
        print("At least 2 hosts required (1 gateway + 1 device).", file=sys.stderr)
        sys.exit(1)

    # Deploy gateway
    simgrid.Actor.create("edge_gateway", gateway_host,
                         EdgeGateway(len(device_hosts)))

    # Deploy edge devices
    for host in device_hosts:
        simgrid.Actor.create("edge_device", host,
                             EdgeDevice(gateway_host.name, 2e8, 1e5))

    e.run()
    simgrid.this_actor.info("=== Simulation completed ===")
    simgrid.this_actor.info(f"Simulated time: {simgrid.Engine.clock:.2f} s")


if __name__ == "__main__":
    main()
