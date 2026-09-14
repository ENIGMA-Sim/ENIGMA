#!/usr/bin/env bash
# Use case 2 – Fog Analytics
# Edge data sources stream samples to fog nodes that run real-time analysis and
# a final aggregate computation (mean / sum) over every received sample.
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

ensure_built platform_generator fog_analytics_app

# 4 fog clusters × 3 nodes = 12 fog hosts; ~1/3 act as analyzers, rest as sources
gen_platform fog-cluster 4 3

run_enigma fog_analytics_app platforms/fog_platform.xml
