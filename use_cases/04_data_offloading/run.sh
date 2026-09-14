#!/usr/bin/env bash
# Use case 4 – Smart Data Offloading
# Every edge device decides at run time whether to process a task locally,
# offload it to the fog, or offload it to the cloud, based on its own CPU
# capacity vs. the task workload. Servers reply with the measured latency.
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

ensure_built platform_generator data_offloading_app

gen_platform hybrid-cluster 2 4 2 2 1 1 1 hybrid_platform.xml

# 2nd arg = number of tasks each edge device generates
run_enigma data_offloading_app platforms/hybrid_platform.xml 5
