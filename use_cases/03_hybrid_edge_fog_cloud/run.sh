#!/usr/bin/env bash
# Use case 3 – Hybrid Edge-Fog-Cloud pipeline
# Three-tier data pipeline:
#   Edge  – collects and filters raw data
#   Fog   – aggregates and pre-processes
#   Cloud – stores and runs the heavy analytics / ML step
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

ensure_built platform_generator hybrid_cloud_app

#            edgeC edgeN  fogC fogN  cloudC cloudN  directEdgeCloud  outfile
gen_platform hybrid-cluster 2 4      1 2    1 1     1                hybrid_platform.xml

run_enigma hybrid_cloud_app platforms/hybrid_platform.xml
