"""
mobility_viewer.py
------------------
Standalone post-simulation mobility viewer (interactive folium map).
Accepts JSON or CSV output from the ENIGMA mobility module.

Usage
-----
    # Open interactive map in browser:
    python3 src/python/tools/mobility_viewer.py snapshots.json

    # Save to specific HTML file:
    python3 src/python/tools/mobility_viewer.py snapshots.csv --save my_map.html

    # Print statistics only (no browser):
    python3 src/python/tools/mobility_viewer.py snapshots.json --stats-only

    # Filter devices:
    python3 src/python/tools/mobility_viewer.py snapshots.json \\
        --devices edge_cluster_0_node_1 edge_cluster_1_node_2

    # Custom map title:
    python3 src/python/tools/mobility_viewer.py snapshots.json --title "My Simulation"

    # Offline mode – embed all CDN JS/CSS so the HTML opens via file:// directly:
    python3 src/python/tools/mobility_viewer.py snapshots.json --offline

Map modes
---------
* **online** (default): HTML references CDN URLs.  Must be served via HTTP,
  not opened as file://.  Start a local server if needed::

      cd /path/to/output && python3 -m http.server 8765
      # then open http://localhost:8765/trajectory_map.html
      # stop the server: kill $(lsof -ti:8765)

* **offline** (``--offline``): downloads and embeds all CDN JS/CSS inline
  (~1.5–2 MB extra per file).  The HTML then works as file:// or HTTP.
  Map tiles (the OpenStreetMap background) are still fetched live at view
  time, so an internet connection is still needed to see the map background.
"""
from __future__ import annotations

import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
_ROOT = os.path.join(_HERE, "..")
if _ROOT not in sys.path:
    sys.path.insert(0, _ROOT)

from enigma.mobility.mobility_visualizer import MobilityVisualizer, _open_browser
from enigma.mobility.mobility_recorder   import MobilityRecorder
from enigma.mobility.mobility_trace      import MobilityPosition


def load_recorder(filepath: str, devices_filter: list | None = None) -> MobilityRecorder:
    """Load a JSON or CSV snapshot file into a MobilityRecorder."""
    ext = os.path.splitext(filepath)[1].lower()
    if ext == ".json":
        rec = MobilityVisualizer._recorder_from_json(filepath)
    elif ext == ".csv":
        rec = MobilityVisualizer._recorder_from_csv(filepath)
    else:
        sys.exit(f"Unsupported file type '{ext}'. Use .json or .csv")

    if devices_filter:
        filtered = MobilityRecorder()
        for name, pos in rec.records():
            if name in devices_filter:
                filtered.add(name, pos)
        return filtered

    return rec


def main() -> None:
    import argparse

    parser = argparse.ArgumentParser(
        description="ENIGMA – Post-simulation interactive mobility map viewer",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    parser.add_argument("input",
                        help="Snapshot file (.json or .csv)")
    parser.add_argument("--save", default=None, metavar="FILE.html",
                        help="Save interactive map to this HTML file "
                             "(default: <input>_map.html)")
    parser.add_argument("--stats-only", action="store_true",
                        help="Print statistics only, skip map generation")
    parser.add_argument("--no-browser", action="store_true",
                        help="Generate the HTML file but do not open a browser")
    parser.add_argument("--offline", action="store_true",
                        help="Embed all CDN JS/CSS inline so the map opens via "
                             "file:// without a local HTTP server. "
                             "Tiles still need internet when viewing.")
    parser.add_argument("--online", dest="offline", action="store_false",
                        help="Use CDN links (default). Requires a local HTTP server "
                             "or internet access.")
    parser.add_argument("--devices", nargs="+", default=None, metavar="NAME",
                        help="Only include these devices (space-separated names)")
    parser.add_argument("--title", default="ENIGMA – Mobility Replay",
                        help="Title shown on the map")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        sys.exit(f"File not found: {args.input}")

    print(f"Loading snapshot file: {args.input}")
    rec = load_recorder(args.input, args.devices)
    print(f"Loaded {rec.count} snapshots for {len(rec.devices())} device(s)")

    rec.print_stats()

    if args.stats_only:
        return

    # Determine output path
    out_path = args.save or (os.path.splitext(os.path.abspath(args.input))[0] + "_map.html")

    MobilityVisualizer.save_interactive_map(
        rec, out_path, title=args.title, offline=args.offline
    )

    if not args.no_browser:
        _open_browser(f"file://{os.path.abspath(out_path)}")
        print(f"Map opened in browser: {out_path}")
    else:
        print(f"Map saved (browser not opened): {out_path}")


if __name__ == "__main__":
    main()
