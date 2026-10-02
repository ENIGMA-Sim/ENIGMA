#!/usr/bin/env bash
# Use case 7 - France high-speed-rail mobility  (step-by-step walkthrough)
#
#   Stage 1  the platform            france_rail_platform.xml
#   Stage 2  the GPS traces          generate_traces.py -> coords/*.csv
#   Stage 3  the simulation          mobility_test_app  (position interpolation + snapshots)
#   Stage 4  energy report           printed by mobility_test_app after the run
#   Stage 5  visualise               mobility_viewer.py / mobility_test.py
#
# 5 TGV run along real LGV corridors, reporting to edge nodes in Paris, Lyon and
# Lille that feed the SNCF supervision fog and the cloud.
#
#   Outputs (in use_cases/07_mobility_france_trains/mobility_output/):
#     france_trains_snapshots.json / .csv   recorded positions + extra columns
#     france_trains_raw_traces.json         full input waypoints per train
#   Set OUT_PREFIX=/some/path/name to write them elsewhere.
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

# --- Stage 3 prerequisite: the mobility-enabled binary --------------------
ensure_built mobility_test_app

# --- Stage 2: (re)generate the SNCF GPS traces --------------------------
python3 "$HERE/generate_traces.py"

OUT_DIR="$HERE/mobility_output"
OUT_PREFIX="$(abs_path "${OUT_PREFIX:-$OUT_DIR/france_trains}")"
mkdir -p "$(dirname "$OUT_PREFIX")"

# --- Stages 3 + 4: run the simulation (prints the energy report) ---------
run_enigma mobility_test_app \
    "$HERE/france_rail_platform.xml" \
    "$OUT_PREFIX" \
    --mobility-dir "$HERE/coords/"

# --- Stage 5: how to visualise ---------------------------------------
# Paths in the hints below are relative to where run.sh was launched from.
OUT="$(rel_path "$OUT_PREFIX")"
cat <<EOF

------------------------------------------------------------------
Recorded snapshots written to:
    ${OUT}_snapshots.json
    ${OUT}_snapshots.csv
    ${OUT}_raw_traces.json

Stage 5 - visualise the run (map -> ${OUT}_snapshots_map.html):

  python3 $(rel_path "$ENIGMA_ROOT/src/python/tools/mobility_viewer.py") ${OUT}_snapshots.json --offline

  # or the full Python pipeline with a live browser map:
  python3 $(rel_path "$ENIGMA_ROOT/src/python/tests/mobility_test.py") \\
      $(rel_path "$HERE/france_rail_platform.xml") \\
      --coords-dir $(rel_path "$HERE/coords")/ \\
      --output $(rel_path "$OUT_DIR/python")
------------------------------------------------------------------
EOF
