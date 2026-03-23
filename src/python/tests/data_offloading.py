"""
data_offloading.py – Adaptive offloading: Edge, Fog, or Cloud based on load.

Mirrors tests/data_offloading.cpp.

Usage
-----
    python data_offloading.py <platform_file.xml>
"""
from __future__ import annotations

import sys
import os

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.join(_HERE, "..")
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

import simgrid


class SmartEdgeDevice:
    """
    Makes runtime offloading decisions:
    - Low workload  → process locally
    - Medium workload → offload to Fog
    - High workload → offload to Cloud
    """

    def __init__(
        self,
        fog_name: str,
        cloud_name: str,
        workload: float,
        num_tasks: int = 1,
    ) -> None:
        self._fog       = fog_name
        self._cloud     = cloud_name
        self._workload  = workload
        self._num_tasks = num_tasks

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        result_mbox_name = f"{host.name}_result"

        simgrid.this_actor.info(
            f"Edge Device '{host.name}' started "
            f"(workload: {self._workload / 1e9:.2f} GFlops, tasks: {self._num_tasks})"
        )

        capacity = host.speed

        for task_id in range(self._num_tasks):
            simgrid.this_actor.info(
                f"--- Processing task {task_id + 1}/{self._num_tasks} ---"
            )

            if self._workload < capacity * 0.5:
                # Local processing
                simgrid.this_actor.info("Decision: LOCAL PROCESSING (low load)")
                t0 = simgrid.Engine.clock
                simgrid.this_actor.execute(self._workload)
                t1 = simgrid.Engine.clock
                simgrid.this_actor.info(f"Local processing completed in {t1 - t0:.2f} s")

            elif self._workload < capacity * 2.0:
                # Offload to Fog
                simgrid.this_actor.info("Decision: OFFLOAD TO FOG (medium load)")
                mbox = simgrid.Mailbox.by_name(self._fog)
                simgrid.this_actor.info(
                    f"Sending task to Fog '{self._fog}' "
                    f"({self._workload / 1e6:.2f} MFlops)…"
                )
                mbox.put((self._workload, result_mbox_name), int(self._workload / 1e3))

                result = simgrid.Mailbox.by_name(result_mbox_name).get()
                simgrid.this_actor.info(f"Result received from Fog: {result}")

            else:
                # Offload to Cloud
                simgrid.this_actor.info("Decision: OFFLOAD TO CLOUD (high load)")
                mbox = simgrid.Mailbox.by_name(self._cloud)
                simgrid.this_actor.info(
                    f"Sending task to Cloud '{self._cloud}' "
                    f"({self._workload / 1e6:.2f} MFlops)…"
                )
                mbox.put((self._workload, result_mbox_name), int(self._workload / 1e3))

                result = simgrid.Mailbox.by_name(result_mbox_name).get()
                simgrid.this_actor.info(f"Result received from Cloud: {result}")

            if task_id < self._num_tasks - 1:
                simgrid.this_actor.sleep_for(0.5)

        simgrid.this_actor.info(f"All {self._num_tasks} tasks completed successfully")


class OffloadingServer:
    """
    Generic compute server (Fog or Cloud) that:
    1. Receives a (workload_flops, return_mailbox) tuple.
    2. Executes the workload.
    3. Sends result back to the caller.
    """

    def __init__(self, server_type: str, max_tasks: int = 10) -> None:
        self._type      = server_type
        self._max_tasks = max_tasks

    def __call__(self) -> None:
        host = simgrid.this_actor.get_host()
        simgrid.this_actor.info(
            f"{self._type} Server '{host.name}' started "
            f"(max_tasks: {self._max_tasks})"
        )

        mbox = simgrid.Mailbox.by_name(host.name)

        tasks_done = 0
        while self._max_tasks < 0 or tasks_done < self._max_tasks:
            try:
                comm = mbox.get_async()
                comm.wait_for(10.0)          # 10 s idle timeout
                item = comm.get_payload()
            except (simgrid.TimeoutException, simgrid.NetworkFailureException):
                break  # No more tasks arriving – shut down

            if item is None:
                break

            tasks_done += 1
            workload, return_mbox = item
            simgrid.this_actor.info(
                f"[{self._type}] Received task ({workload / 1e6:.2f} MFlops) "
                f"from '{return_mbox}'"
            )

            simgrid.this_actor.execute(workload)

            result = (
                f"Completed {workload / 1e6:.2f} MFlops on "
                f"{self._type} server '{host.name}' at "
                f"t={simgrid.Engine.clock:.2f} s"
            )
            simgrid.this_actor.info(f"[{self._type}] Sending result to '{return_mbox}'")
            simgrid.Mailbox.by_name(return_mbox).put(result, 1_000)

        simgrid.this_actor.info(f"{self._type} Server completed")


def main() -> None:
    e = simgrid.Engine(sys.argv)

    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <platform_file.xml>", file=sys.stderr)
        sys.exit(1)

    e.load_platform(sys.argv[1])
    hosts = e.all_hosts

    simgrid.this_actor.info("=== Data Offloading Application ===")
    simgrid.this_actor.info(f"Platform loaded with {len(hosts)} hosts")

    edge_hosts  = [h for h in hosts if "edge"  in h.name]
    fog_hosts   = [h for h in hosts if "fog"   in h.name]
    cloud_hosts = [h for h in hosts if "cloud" in h.name]

    # Fallbacks when naming convention is not used
    if not edge_hosts and not fog_hosts and not cloud_hosts:
        if len(hosts) < 3:
            print("Need at least 3 hosts.", file=sys.stderr)
            sys.exit(1)
        edge_hosts  = list(hosts[2:])
        fog_hosts   = [hosts[1]]
        cloud_hosts = [hosts[0]]

    if not fog_hosts or not cloud_hosts:
        print("Platform must have at least one fog and one cloud host.", file=sys.stderr)
        sys.exit(1)

    fog_host   = fog_hosts[0]
    cloud_host = cloud_hosts[0]

    simgrid.this_actor.info(
        f"Servers: fog='{fog_host.name}', cloud='{cloud_host.name}'"
    )
    simgrid.this_actor.info(f"Edge devices: {[h.name for h in edge_hosts]}")

    # Deploy servers
    simgrid.Actor.create("fog_server",   fog_host,   OffloadingServer("FOG",   max_tasks=-1))
    simgrid.Actor.create("cloud_server", cloud_host, OffloadingServer("CLOUD", max_tasks=-1))

    # Deploy edge devices with varying workloads
    workloads = [5e8, 5e9, 5e10]   # low, medium, high
    for i, host in enumerate(edge_hosts):
        wl = workloads[i % len(workloads)]
        simgrid.Actor.create("smart_edge", host,
                             SmartEdgeDevice(fog_host.name, cloud_host.name,
                                             wl, num_tasks=3))

    e.run()
    simgrid.this_actor.info("=== Simulation completed ===")
    simgrid.this_actor.info(f"Simulated time: {simgrid.Engine.clock:.2f} s")


if __name__ == "__main__":
    main()
