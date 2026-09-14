#!/usr/bin/env bash
# Use case 6 - Madrid public-transport mobility
# EMT buses, Metro / Cercanias trains and inspection drones move along real
# Madrid routes while reporting to edge nodes at Atocha and Nuevos Ministerios.
# ENIGMA's mobility module attaches the GPS traces to the SimGrid hosts,
# interpolates each device's position during the run, records periodic
# snapshots and exports them for the interactive map.
#
#   Outputs (prefix /tmp/enigma_uc6_*):
#     _snapshots.json / _snapshots.csv   recorded positions + extra columns
#     _raw_traces.json                    full input waypoints per device
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

ensure_built mobility_test_app

# (Re)generate the Madrid GPS traces - pure stdlib, safe to run every time.
python3 "$HERE/generate_traces.py"

OUT_PREFIX="${OUT_PREFIX:-/tmp/enigma_uc6}"

run_enigma mobility_test_app \
    "$HERE/madrid_transport_platform.xml" \
    "$OUT_PREFIX" \
    --mobility-dir "$HERE/coords/"

cat <<EOF

------------------------------------------------------------------
Recorded snapshots written to:
    ${OUT_PREFIX}_snapshots.json
    ${OUT_PREFIX}_snapshots.csv
    ${OUT_PREFIX}_raw_traces.json

Interactive map (needs: pip install folium playwright && playwright install chromium):
    python3 src/python/tools/mobility_viewer.py ${OUT_PREFIX}_snapshots.json --offline

Full Python pipeline with a live browser map during the simulation:
    python3 src/python/tests/mobility_test.py \\
        use_cases/06_mobility_madrid/madrid_transport_platform.xml \\
        --coords-dir use_cases/06_mobility_madrid/coords/
------------------------------------------------------------------
EOF
