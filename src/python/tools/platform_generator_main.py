#!/usr/bin/env python3
"""
platform_generator_main.py – CLI tool for ENIGMA Python platform generation.

Mirrors src/tools/platform_generator_main.cpp.

Usage
-----
python platform_generator_main.py <type> [options]

Platform types
--------------
  Simple (all hosts interconnected):
    edge    <num_devices>                         - Edge star topology
    fog     <num_nodes>                           - Fog hierarchical topology
    cloud   <num_servers>                         - Cloud cluster
    iot     <sensors> <actuators>                 - IoT platform

  Cluster-based:
    edge-cluster   <num_clusters> <nodes_per_cluster>
    fog-cluster    <num_clusters> <nodes_per_cluster>
    cloud-cluster  <num_clusters> <nodes_per_cluster>
    hybrid-cluster <edge_clusters> <edge_nodes>
                   <fog_clusters>  <fog_nodes>
                   <cloud_clusters> <cloud_nodes>
                   [edge_cloud_direct] [output_file] [--generate-app]

Examples
--------
  python platform_generator_main.py edge 10
  python platform_generator_main.py edge-cluster 3 5
  python platform_generator_main.py hybrid-cluster 2 10 2 5 1 20
  python platform_generator_main.py hybrid-cluster 2 10 2 5 1 20 1 my_hybrid.xml --generate-app
"""
from __future__ import annotations

import argparse
import os
import sys
import textwrap

# Make sure the package on src/python is importable when run directly
_HERE = os.path.dirname(os.path.abspath(__file__))
_PYTHON_ROOT = os.path.join(_HERE, "..")
if _PYTHON_ROOT not in sys.path:
    sys.path.insert(0, _PYTHON_ROOT)

from enigma.platform import (
    ClusterConfig,
    PlatformGenerator,
    EdgePlatform,
    FogPlatform,
    CloudPlatform,
)


# --------------------------------------------------------------------------- #
# Template-app generator                                                       #
# --------------------------------------------------------------------------- #

def generate_template_app(
    app_filename: str,
    platform_file: str,
    edge_clusters: int, edge_nodes: int,
    fog_clusters: int,  fog_nodes: int,
    cloud_clusters: int, cloud_nodes: int,
) -> None:
    """Generate a Python template application skeleton for a given platform layout."""
    os.makedirs(os.path.dirname(os.path.abspath(app_filename)), exist_ok=True)

    lines: list[str] = [
        '"""',
        f'Template application for platform: {platform_file}',
        '',
        f'Configuration:',
        f'  Edge clusters : {edge_clusters} × {edge_nodes} nodes',
        f'  Fog  clusters : {fog_clusters} × {fog_nodes} nodes',
        f'  Cloud clusters: {cloud_clusters} × {cloud_nodes} nodes',
        '',
        'TODO: Implement actor logic in each class below.',
        '"""',
        '',
        'import sys',
        'import simgrid',
        '',
    ]

    if edge_clusters > 0:
        lines += [
            '',
            'class EdgeDevice:',
            '    """Edge device actor – sense, filter, forward."""',
            '',
            '    def __call__(self) -> None:',
            '        host = simgrid.this_actor.get_host()',
            '        simgrid.this_actor.info(f"[EDGE] Device \'{host.name}\' started")',
            '        # TODO: add edge processing logic here',
            '        # Example: collect data, execute locally, send to fog',
            '        simgrid.this_actor.info(f"[EDGE] Device \'{host.name}\' finished")',
            '',
        ]

    if fog_clusters > 0:
        lines += [
            '',
            'class FogNode:',
            '    """Fog node actor – aggregate, pre-process, forward."""',
            '',
            '    def __call__(self) -> None:',
            '        host = simgrid.this_actor.get_host()',
            '        simgrid.this_actor.info(f"[FOG] Node \'{host.name}\' started")',
            '        # TODO: add fog processing logic here',
            '        # Example: receive from edge, aggregate, filter, forward to cloud',
            '        simgrid.this_actor.info(f"[FOG] Node \'{host.name}\' finished")',
            '',
        ]

    if cloud_clusters > 0:
        lines += [
            '',
            'class CloudServer:',
            '    """Cloud server actor – heavy analytics, storage."""',
            '',
            '    def __call__(self) -> None:',
            '        host = simgrid.this_actor.get_host()',
            '        simgrid.this_actor.info(f"[CLOUD] Server \'{host.name}\' started")',
            '        # TODO: add cloud processing logic here',
            '        # Example: receive summaries, perform ML, store results',
            '        simgrid.this_actor.info(f"[CLOUD] Server \'{host.name}\' finished")',
            '',
        ]

    lines += [
        '',
        'def main() -> None:',
        '    e = simgrid.Engine(sys.argv)',
        '',
        '    if len(sys.argv) < 2:',
        '        print(f"Usage: {sys.argv[0]} <platform_file.xml>")',
        '        sys.exit(1)',
        '',
        '    e.load_platform(sys.argv[1])',
        '',
        '    hosts = e.all_hosts',
        '    simgrid.this_actor.info("=== Application Template ===")',
        '    simgrid.this_actor.info(f"Platform loaded with {len(hosts)} hosts")',
        '',
        '    edge_hosts  = [h for h in hosts if "edge"  in h.name]',
        '    fog_hosts   = [h for h in hosts if "fog"   in h.name]',
        '    cloud_hosts = [h for h in hosts if "cloud" in h.name]',
        '',
        '    simgrid.this_actor.info(',
        '        f"Detected: {len(edge_hosts)} edge, {len(fog_hosts)} fog, '
        '{len(cloud_hosts)} cloud hosts"',
        '    )',
        '',
    ]

    if edge_clusters > 0:
        lines += [
            '    for host in edge_hosts:',
            '        simgrid.Actor.create("edge_device", host, EdgeDevice())',
            '',
        ]
    if fog_clusters > 0:
        lines += [
            '    for host in fog_hosts:',
            '        simgrid.Actor.create("fog_node", host, FogNode())',
            '',
        ]
    if cloud_clusters > 0:
        lines += [
            '    for host in cloud_hosts:',
            '        simgrid.Actor.create("cloud_server", host, CloudServer())',
            '',
        ]

    lines += [
        '    e.run()',
        '    simgrid.this_actor.info("=== Simulation completed ===")',
        f'    simgrid.this_actor.info(f"Simulated time: {{simgrid.Engine.clock:.2f}} s")',
        '',
        '',
        'if __name__ == "__main__":',
        '    main()',
    ]

    with open(app_filename, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    print(f"Template application generated: {app_filename}")
    print("\nNext steps:")
    print(f"  1. Edit {app_filename} and implement your actor logic")
    print(f"  2. Run: python {app_filename} {platform_file}")


# --------------------------------------------------------------------------- #
# Main CLI                                                                     #
# --------------------------------------------------------------------------- #

def print_usage() -> None:
    print(textwrap.dedent(__doc__ or "").strip())


def main(argv: list[str] | None = None) -> int:  # noqa: C901
    argv = argv or sys.argv[1:]

    if not argv:
        print_usage()
        return 1

    platform_type = argv[0]
    gen = PlatformGenerator()

    try:
        # ------------------------------------------------------------------ #
        # Simple (host-based) platforms                                       #
        # ------------------------------------------------------------------ #
        if platform_type == "edge" and len(argv) >= 2:
            n = int(argv[1])
            print(f"Generating Edge platform with {n} devices…")
            zone = EdgePlatform.create_star_topology(n)
            gen.generate_platform("platforms/edge_platform.xml", zone)

        elif platform_type == "fog" and len(argv) >= 2:
            n = int(argv[1])
            print(f"Generating Fog platform with {n} nodes…")
            zone = FogPlatform.create_hierarchical_topology(n)
            gen.generate_platform("platforms/fog_platform.xml", zone)

        elif platform_type == "cloud" and len(argv) >= 2:
            n = int(argv[1])
            print(f"Generating Cloud cluster with {n} servers…")
            zone = CloudPlatform.create_cluster(n)
            gen.generate_platform("platforms/cloud_platform.xml", zone)

        elif platform_type == "iot" and len(argv) >= 3:
            sensors    = int(argv[1])
            actuators  = int(argv[2])
            print(f"Generating IoT platform: {sensors} sensors, {actuators} actuators…")
            zone = EdgePlatform.create_iot_platform(sensors, actuators)
            gen.generate_platform("platforms/iot_platform.xml", zone)

        # ------------------------------------------------------------------ #
        # Cluster-based platforms                                             #
        # ------------------------------------------------------------------ #
        elif platform_type == "edge-cluster" and len(argv) >= 3:
            num_clusters = int(argv[1])
            nodes        = int(argv[2])
            print(f"Generating Edge cluster platform: {num_clusters} clusters × {nodes} nodes…")
            clusters = [
                ClusterConfig(f"edge_cluster_{i}", nodes, "1Gf", 1, "125MBps", "50us")
                for i in range(num_clusters)
            ]
            zone = PlatformGenerator.create_edge_with_clusters("edge_platform", clusters)
            gen.generate_platform("platforms/edge_platform.xml", zone)

        elif platform_type == "fog-cluster" and len(argv) >= 3:
            num_clusters = int(argv[1])
            nodes        = int(argv[2])
            print(f"Generating Fog cluster platform: {num_clusters} clusters × {nodes} nodes…")
            clusters = [
                ClusterConfig(f"fog_cluster_{i}", nodes, "10Gf", 4, "1GBps", "10us")
                for i in range(num_clusters)
            ]
            zone = PlatformGenerator.create_fog_with_clusters("fog_platform", clusters)
            gen.generate_platform("platforms/fog_platform.xml", zone)

        elif platform_type == "cloud-cluster" and len(argv) >= 3:
            num_clusters = int(argv[1])
            nodes        = int(argv[2])
            print(f"Generating Cloud cluster platform: {num_clusters} clusters × {nodes} nodes…")
            clusters = [
                ClusterConfig(f"cloud_cluster_{i}", nodes, "100Gf", 16, "10GBps", "1us")
                for i in range(num_clusters)
            ]
            zone = PlatformGenerator.create_cloud_with_clusters("cloud_platform", clusters)
            gen.generate_platform("platforms/cloud_platform.xml", zone)

        elif platform_type == "hybrid-cluster" and len(argv) >= 7:
            edge_c = int(argv[1])
            edge_n = int(argv[2])
            fog_c  = int(argv[3])
            fog_n  = int(argv[4])
            cloud_c = int(argv[5])
            cloud_n = int(argv[6])

            # Parse optional trailing arguments
            edge_cloud_direct = False
            output_file = "platforms/hybrid_platform.xml"
            generate_app = False

            for arg in argv[7:]:
                if arg == "--generate-app":
                    generate_app = True
                elif arg.lstrip("-+").isdigit():
                    edge_cloud_direct = bool(int(arg))
                else:
                    output_file = arg if "/" in arg else f"platforms/{arg}"

            print("Generating flat hybrid platform:")
            print(f"  Edge  : {edge_c} clusters × {edge_n} nodes")
            print(f"  Fog   : {fog_c} clusters × {fog_n} nodes")
            print(f"  Cloud : {cloud_c} clusters × {cloud_n} nodes")
            print(f"  Direct Edge-Cloud: {'YES' if edge_cloud_direct else 'NO'}")
            print(f"  Output: {output_file}")

            edge_clusters = [
                ClusterConfig(f"edge_cluster_{i}", edge_n, "1Gf", 1, "125MBps", "50us")
                for i in range(edge_c)
            ]
            fog_clusters = [
                ClusterConfig(f"fog_cluster_{i}", fog_n, "10Gf", 4, "1GBps", "10us")
                for i in range(fog_c)
            ]
            cloud_clusters = [
                ClusterConfig(f"cloud_cluster_{i}", cloud_n, "100Gf", 16, "10GBps", "1us")
                for i in range(cloud_c)
            ]

            zone = PlatformGenerator.create_hybrid_with_clusters_flat(
                edge_clusters, fog_clusters, cloud_clusters, edge_cloud_direct
            )
            gen.generate_platform(output_file, zone)

            if generate_app:
                base = os.path.splitext(os.path.basename(output_file))[0]
                app_file = os.path.join("tests", f"{base}_app.py")
                generate_template_app(
                    app_file, output_file,
                    edge_c, edge_n,
                    fog_c, fog_n,
                    cloud_c, cloud_n,
                )

        else:
            print(f"Error: unrecognised command or missing arguments: {argv}", file=sys.stderr)
            print_usage()
            return 1

    except (ValueError, IndexError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
