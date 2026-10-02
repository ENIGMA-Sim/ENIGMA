#!/usr/bin/env bash
# Use case 6 - Madrid public-transport mobility
# EMT buses, Metro / Cercanias trains and inspection drones move along real
# Madrid routes while reporting to edge nodes at Atocha and Nuevos Ministerios.
# ENIGMA's mobility module attaches the GPS traces to the SimGrid hosts,
# interpolates each device's position during the run, records periodic
# snapshots and exports them for the interactive map.
#
#   Outputs (in use_cases/06_mobility_madrid/mobility_output/):
#     madrid_snapshots.json / .csv   recorded positions + extra columns
#     madrid_raw_traces.json         full input waypoints per device
#   Set OUT_PREFIX=/some/path/name to write them elsewhere.
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

ensure_built mobility_test_app

# (Re)generate the Madrid GPS traces - pure stdlib, safe to run every time.
python3 "$HERE/generate_traces.py"

OUT_DIR="$HERE/mobility_output"
OUT_PREFIX="$(abs_path "${OUT_PREFIX:-$OUT_DIR/madrid}")"
mkdir -p "$(dirname "$OUT_PREFIX")"

run_enigma mobility_test_app \
    "$HERE/madrid_transport_platform.xml" \
    "$OUT_PREFIX" \
    --mobility-dir "$HERE/coords/"

# Paths in the hints below are relative to where run.sh was launched from.
OUT="$(rel_path "$OUT_PREFIX")"
cat <<EOF

------------------------------------------------------------------
Recorded snapshots written to:
    ${OUT}_snapshots.json
    ${OUT}_snapshots.csv
    ${OUT}_raw_traces.json

Interactive map (needs: pip install folium) -> ${OUT}_snapshots_map.html:
    python3 $(rel_path "$ENIGMA_ROOT/src/python/tools/mobility_viewer.py") ${OUT}_snapshots.json --offline

Full Python pipeline with a live browser map during the simulation:
    python3 $(rel_path "$ENIGMA_ROOT/src/python/tests/mobility_test.py") \\
        $(rel_path "$HERE/madrid_transport_platform.xml") \\
        --coords-dir $(rel_path "$HERE/coords")/ \\
        --output $(rel_path "$OUT_DIR/python")
------------------------------------------------------------------
EOF
