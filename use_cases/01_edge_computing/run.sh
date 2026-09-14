#!/usr/bin/env bash
# Use case 1 – Edge Computing
# Edge devices collect sensor data, process it locally and push the result to
# an edge gateway that aggregates everything.
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

ensure_built platform_generator edge_computing_app

# 3 edge clusters × 5 nodes = 15 edge hosts (one is picked as the gateway)
gen_platform edge-cluster 3 5

run_enigma edge_computing_app platforms/edge_platform.xml
