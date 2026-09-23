#!/usr/bin/env bash
# Use case 7 - France high-speed-rail mobility  (step-by-step walkthrough)
#
#   Stage 1  the platform            france_rail_platform.xml
#   Stage 2  the GPS traces          generate_traces.py -> coords/*.csv
#   Stage 3  the simulation          mobility_test_app  (position interpolation + snapshots)
#   Stage 4  visualise               mobility_viewer.py / mobility_test.py
#
# 5 TGV run along real LGV corridors, reporting to edge nodes in Paris, Lyon and
# Lille that feed the SNCF supervision fog and the cloud.
set -euo pipefail
HERE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "$HERE/../common.sh"

# --- Stage 3 prerequisite: the mobility-enabled binary --------------------
ensure_built mobility_test_app

# --- Stage 2: (re)generate the SNCF GPS traces --------------------------
python3 "$HERE/generate_traces.py"

OUT_PREFIX="${OUT_PREFIX:-/tmp/enigma_uc7}"

# --- Stage 3: run the simulation --------------------------------------
run_enigma mobility_test_app \
    "$HERE/france_rail_platform.xml" \
    "$OUT_PREFIX" \
    --mobility-dir "$HERE/coords/"

# --- Stage 4: how to visualize ---------------------------------------
cat <<EOF

------------------------------------------------------------------
Stage 4 - visualize the run:

  python3 src/python/tools/mobility_viewer.py ${OUT_PREFIX}_snapshots.json --offline

  # or the full Python pipeline with a live browser map:
  python3 src/python/tests/mobility_test.py \\
      use_cases/07_mobility_france_trains/france_rail_platform.xml \\
      --coords-dir use_cases/07_mobility_france_trains/coords/
------------------------------------------------------------------
EOF
