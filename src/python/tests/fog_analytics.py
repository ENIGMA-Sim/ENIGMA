"""
fog_analytics.py – Edge data sources feed a Fog node for real-time analytics.

Mirrors tests/fog_analytics.cpp.

Usage
-----
    python fog_analytics.py <platform_file.xml>
"""
from __future__ import annotations

import sys
import os
import random

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.join(_HERE, "..")
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

import simgrid

_SENTINEL = "__FIN__"


class DataSource:
    """Generates periodic sensor samples and pushes them to a Fog node."""

    def __init__(self, fog_node: str, num_samples: int = 5) -> None:
        self._fog       = fog_node
        self._num       = num_samples

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(f"Data source '{host.name}' started")

        mbox = simgrid.Mailbox.by_name(self._fog)
        rng  = random.Random(id(self))

        for i in range(self._num):
            wait = rng.uniform(0.5, 2.0)
            simgrid.this_actor.sleep_for(wait)
            value = round(100.0 * rng.uniform(0.5, 2.0), 3)
            simgrid.this_actor.info(f"Sending sample #{i + 1} (value={value})")
            mbox.put(value, 10_000)    # 10 KB

        # Send completion sentinel
        mbox.put(_SENTINEL, 100)
        simgrid.this_actor.info("All samples sent")


class FogAnalyzer:
    """Receives samples from multiple data sources and performs analytics."""

    def __init__(self, num_sources: int) -> None:
        self._num_sources = num_sources

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(f"Fog node '{host.name}' started for analysis")

        mbox = simgrid.Mailbox.by_name(host.name)
        data: list[float] = []
        finished = 0

        while finished < self._num_sources:
            msg = mbox.get()

            if msg == _SENTINEL:
                finished += 1
                simgrid.this_actor.info(
                    f"Source completed ({finished}/{self._num_sources})"
                )
                continue

            # msg is a float sensor value
            data.append(float(msg))
            simgrid.this_actor.execute(1e8)   # 100 MFlops per sample

        simgrid.this_actor.info(
            f"Performing aggregate analysis of {len(data)} samples…"
        )
        simgrid.this_actor.execute(len(data) * 5e7)

        if data:
            mean = sum(data) / len(data)
            minv, maxv = min(data), max(data)
            simgrid.this_actor.info(
                f"Analysis results – mean: {mean:.2f}, "
                f"min: {minv:.2f}, max: {maxv:.2f}"
            )

        simgrid.this_actor.info("Fog analysis completed")


def main() -> None:
    e = simgrid.Engine(sys.argv)

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <platform_file.xml>", file=sys.stderr)
        sys.exit(1)

    e.load_platform(sys.argv[1])
    hosts = e.all_hosts

    simgrid.this_actor.info("=== Fog Analytics Application ===")
    simgrid.this_actor.info(f"Platform loaded with {len(hosts)} hosts")

    fog_hosts  = [h for h in hosts if "fog"  in h.name]
    edge_hosts = [h for h in hosts if "edge" in h.name]

    if not fog_hosts:
        fog_hosts = [hosts[0]]
    if not edge_hosts:
        edge_hosts = hosts[1:] if len(hosts) > 1 else []

    if not edge_hosts:
        print("Need at least 1 fog host + 1 edge host.", file=sys.stderr)
        sys.exit(1)

    fog_host = fog_hosts[0]
    simgrid.this_actor.info(f"Fog analyzer on '{fog_host.name}'")

    simgrid.Actor.create("fog_analyzer", fog_host,
                         FogAnalyzer(len(edge_hosts)))

    for host in edge_hosts:
        simgrid.Actor.create("data_source", host,
                             DataSource(fog_host.name, num_samples=5))

    e.run()
    simgrid.this_actor.info("=== Simulation completed ===")
    simgrid.this_actor.info(f"Simulated time: {simgrid.Engine.clock:.2f} s")


if __name__ == "__main__":
    main()
