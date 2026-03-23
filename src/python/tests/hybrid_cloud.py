"""
hybrid_cloud.py – Edge-Fog-Cloud three-tier simulation.

Mirrors tests/hybrid_cloud.cpp.

Usage
-----
    python hybrid_cloud.py <platform_file.xml>

Example
-------
    python hybrid_cloud.py platforms/hybrid_platform.xml
"""
from __future__ import annotations

import sys
import simgrid


# --------------------------------------------------------------------------- #
# Actors                                                                       #
# --------------------------------------------------------------------------- #

class EdgeCollector:
    """Edge device: collects sensor data, filters locally, sends to Fog."""

    def __init__(self, fog_mailbox: str) -> None:
        self._fog_mbox = fog_mailbox

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(f"[EDGE] '{host.name}' started")

        mbox = simgrid.Mailbox.by_name(self._fog_mbox)

        for i in range(3):
            simgrid.this_actor.sleep_for(1.0)
            simgrid.this_actor.info("[EDGE] Filtering data locally…")
            simgrid.this_actor.execute(2e8)   # 200 MFlops

            data = f"edge_data_{i}_from_{host.name}"
            simgrid.this_actor.info(f"[EDGE] Sending data to Fog '{self._fog_mbox}'")
            mbox.put(data, 500_000)           # 500 KB

        simgrid.this_actor.info("[EDGE] Collection completed")


class FogAggregator:
    """Fog node: receives Edge data, aggregates, forwards summary to Cloud."""

    def __init__(self, cloud_mailbox: str, num_edge_devices: int) -> None:
        self._cloud_mbox = cloud_mailbox
        self._num_edge = num_edge_devices

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(f"[FOG] '{host.name}' started")

        mbox = simgrid.Mailbox.by_name(host.name)
        aggregated: list[str] = []
        expected = self._num_edge * 3   # 3 messages per device

        while len(aggregated) < expected:
            data = mbox.get()
            aggregated.append(data)
            simgrid.this_actor.info(
                f"[FOG] Data received ({len(aggregated)}/{expected})"
            )
            simgrid.this_actor.execute(5e8)   # 500 MFlops

        simgrid.this_actor.info(
            f"[FOG] Performing aggregation of {len(aggregated)} data sets…"
        )
        simgrid.this_actor.execute(2e9)   # 2 GFlops

        summary = f"fog_summary_{len(aggregated)}_items"
        simgrid.this_actor.info(f"[FOG] Sending summary to Cloud '{self._cloud_mbox}'")
        simgrid.Mailbox.by_name(self._cloud_mbox).put(summary, 1_000_000)  # 1 MB

        simgrid.this_actor.info("[FOG] Processing completed")


class CloudProcessor:
    """Cloud server: receives Fog summaries, performs heavy analytics."""

    def __init__(self, num_fog_nodes: int) -> None:
        self._num_fog = num_fog_nodes

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(f"[CLOUD] '{host.name}' started")

        mbox = simgrid.Mailbox.by_name(host.name)

        for i in range(self._num_fog):
            summary = mbox.get()
            simgrid.this_actor.info(f"[CLOUD] Received summary: {summary}")
            simgrid.this_actor.execute(1e9)   # 1 GFlop per summary

        simgrid.this_actor.info("[CLOUD] Performing full analysis and machine learning…")
        simgrid.this_actor.execute(1e10)  # 10 GFlops

        simgrid.this_actor.info(
            f"[CLOUD] Analysis completed at t={simgrid.Engine.clock:.2f} s"
        )


# --------------------------------------------------------------------------- #
# main                                                                         #
# --------------------------------------------------------------------------- #

def main() -> None:
    e = simgrid.Engine(sys.argv)

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <platform_file.xml>", file=sys.stderr)
        sys.exit(1)

    e.load_platform(sys.argv[1])
    hosts = e.all_hosts

    simgrid.this_actor.info("=== Hybrid Cloud Application ===")
    simgrid.this_actor.info(f"Platform loaded with {len(hosts)} hosts")

    # Classify hosts
    edge_hosts  = sorted([h for h in hosts if "edge"  in h.name], key=lambda h: h.name)
    fog_hosts   = sorted([h for h in hosts if "fog"   in h.name], key=lambda h: h.name)
    cloud_hosts = sorted([h for h in hosts if "cloud" in h.name], key=lambda h: h.name)

    simgrid.this_actor.info(
        f"Detected: {len(edge_hosts)} edge, {len(fog_hosts)} fog, "
        f"{len(cloud_hosts)} cloud hosts"
    )

    if not edge_hosts or not fog_hosts or not cloud_hosts:
        print(
            "Error: platform must have edge, fog AND cloud hosts.",
            file=sys.stderr,
        )
        sys.exit(1)

    fog_host  = fog_hosts[0]
    cloud_host = cloud_hosts[0]

    # Deploy Fog aggregator (uses its own host name as mailbox)
    simgrid.Actor.create("fog_aggregator", fog_host,
                         FogAggregator(cloud_host.name, len(edge_hosts)))

    # Deploy Cloud processor (expects exactly 1 summary from the 1 deployed fog aggregator)
    simgrid.Actor.create("cloud_processor", cloud_host,
                         CloudProcessor(1))

    # Deploy Edge collectors
    for host in edge_hosts:
        simgrid.Actor.create("edge_collector", host,
                             EdgeCollector(fog_host.name))

    e.run()
    simgrid.this_actor.info("=== Simulation completed ===")
    simgrid.this_actor.info(f"Simulated time: {simgrid.Engine.clock:.2f} s")


if __name__ == "__main__":
    main()
