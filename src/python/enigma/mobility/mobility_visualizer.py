"""
mobility_visualizer.py
----------------------
Interactive map visualization of device mobility using **folium** (OpenStreetMap).

Two usage modes
~~~~~~~~~~~~~~~
1. **Post-simulation** – call :func:`save_interactive_map` (or
   ``MobilityVisualizer.replay_from_recorder``) after ``e.run()`` to generate
   a self-contained HTML file you can open in any browser.

   The map contains:
   - Coloured trajectory lines for every device.
   - An animated time slider (``TimestampedGeoJson``) that moves dots along
     their trajectories frame by frame.
   - A **popup on every dot** showing *all* recorded stats for that snapshot
     (whatever columns were in the CSV – no assumed schema).
   - Start ▶ / End ■ markers with info popups.
   - Auto-fit bounds and a colour legend.

2. **Live (during simulation)** – instantiate :class:`MobilityVisualizer`,
   call ``start()``, then pass it to ``MobilityManager``.  A background
   thread rebuilds the HTML every *update_interval_s* seconds; the page has
   an auto-refresh meta-tag so an open browser tab updates itself.

Usage
-----
    # Post-sim only
    from enigma.mobility import MobilityVisualizer
    MobilityVisualizer.save_interactive_map(recorder, "mobility_map.html")

    # Live + post-sim
    viz = MobilityVisualizer(live_path="/tmp/mobility_live.html",
                             update_interval_s=3.0, title="My Sim")
    viz.start()                         # opens browser
    mob = MobilityManager(e, coords_dir="...", visualizer=viz)
    mob.start_periodic_actor(e, 0.5)
    e.run()
    viz.stop()
    viz.replay_from_recorder(mob.recorder, save_path="final_map.html")
"""
from __future__ import annotations

import os
import queue
import subprocess
import sys
import threading
import time
import webbrowser
from datetime import datetime, timedelta
from typing import Dict, List, Optional

from .mobility_trace    import MobilityPosition
from .mobility_recorder import MobilityRecorder


# ─────────────────────────────────────────────────────────────────────────── #
# Colour palette
# ─────────────────────────────────────────────────────────────────────────── #

_COLOURS = [
    "#e6194b", "#3cb44b", "#4363d8", "#f58231", "#911eb4",
    "#42d4f4", "#f032e6", "#bfef45", "#469990", "#ffe119",
    "#dcbeff", "#9a6324", "#800000", "#aaffc3", "#000075",
    "#ffd8b1", "#808000", "#fabed4", "#a9a9a9", "#ffffff",
]

# Epoch used to convert simulation seconds → ISO datetime for the time slider
_SIM_EPOCH = datetime(2000, 1, 1, 0, 0, 0)


def _colour(name: str, colour_map: Dict[str, str]) -> str:
    if name not in colour_map:
        colour_map[name] = _COLOURS[len(colour_map) % len(_COLOURS)]
    return colour_map[name]


def _open_browser(url: str) -> None:
    """
    Open *url* in the system browser.

    On Linux, uses ``xdg-open`` with stdout/stderr redirected to DEVNULL to
    suppress Snap / AppArmor / Chromium noise.

    If *url* is a ``file://`` path under ``/tmp``, the file is first copied
    to ``~/.cache/enigma/`` because Snap-packaged browsers (Brave, Chrome)
    have AppArmor policies that deny access to ``/tmp`` via ``file://``.
    """
    import shutil
    # Relocate /tmp files so Snap browsers can open them.
    # If the copy fails for any reason, fall back to the original path.
    if url.startswith("file:///tmp/"):
        try:
            src_path  = url[len("file://"):]
            cache_dir = os.path.join(os.path.expanduser("~"), ".cache", "enigma")
            os.makedirs(cache_dir, exist_ok=True)
            dst_path  = os.path.join(cache_dir, os.path.basename(src_path))
            shutil.copy2(src_path, dst_path)
            url = f"file://{dst_path}"
            print(f"[MobilityVisualizer] Snap workaround: opening from {dst_path}")
        except Exception:
            pass  # could not copy – open the original /tmp path directly

    if sys.platform.startswith("linux"):
        try:
            subprocess.Popen(
                ["xdg-open", url],
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
            return
        except FileNotFoundError:
            pass
    webbrowser.open(url)


def _embed_cdn_resources(html: str) -> str:
    """
    Download every external JS / CSS CDN resource referenced in *html* and
    replace the tags with inlined content.

    For CSS files, ``url()`` references to fonts and images (relative paths,
    protocol-relative ``//`` URLs, or absolute ``https://`` URLs) are also
    fetched and converted to base64 data URIs so the file works as
    ``file://`` without any broken assets.

    Map *tiles* (loaded dynamically by Leaflet at pan/zoom time) still
    require internet access.
    """
    import re
    import base64
    import urllib.request
    import urllib.parse
    import mimetypes

    _text_cache:   Dict[str, str]   = {}
    _binary_cache: Dict[str, bytes] = {}

    _MIME_MAP = {
        ".eot":   "application/vnd.ms-fontobject",
        ".woff":  "font/woff",
        ".woff2": "font/woff2",
        ".ttf":   "font/ttf",
        ".otf":   "font/otf",
        ".svg":   "image/svg+xml",
        ".png":   "image/png",
        ".gif":   "image/gif",
        ".jpg":   "image/jpeg",
        ".jpeg":  "image/jpeg",
    }

    def _mime(url: str) -> str:
        path = url.split("?")[0].split("#")[0]
        ext  = os.path.splitext(path)[1].lower()
        return _MIME_MAP.get(ext, mimetypes.guess_type(path)[0] or "application/octet-stream")

    def _fetch_text(url: str) -> str:
        if url not in _text_cache:
            try:
                req = urllib.request.Request(
                    url, headers={"User-Agent": "Python/ENIGMA-mobility"}
                )
                with urllib.request.urlopen(req, timeout=20) as resp:
                    _text_cache[url] = resp.read().decode("utf-8", errors="replace")
                print(f"[MobilityVisualizer] offline: embedded {url}")
            except Exception as exc:
                print(f"[MobilityVisualizer] offline: WARNING could not fetch {url}: {exc}")
                _text_cache[url] = f"/* failed: {url} */"
        return _text_cache[url]

    def _fetch_binary_data_uri(url: str) -> str:
        """Fetch a binary resource and return a base64 data URI."""
        if url not in _binary_cache:
            try:
                req = urllib.request.Request(
                    url, headers={"User-Agent": "Python/ENIGMA-mobility"}
                )
                with urllib.request.urlopen(req, timeout=20) as resp:
                    _binary_cache[url] = resp.read()
                print(f"[MobilityVisualizer] offline: embedded (binary) {url}")
            except Exception as exc:
                print(f"[MobilityVisualizer] offline: WARNING binary fetch failed {url}: {exc}")
                _binary_cache[url] = b""
        raw = _binary_cache[url]
        if not raw:
            return url
        enc = base64.b64encode(raw).decode("ascii")
        return f"data:{_mime(url)};base64,{enc}"

    def _resolve(href: str, base_url: str) -> str:
        """Resolve a possibly-relative or protocol-relative URL against base_url."""
        if href.startswith("//"):
            return "https:" + href
        if href.startswith("http://") or href.startswith("https://"):
            return href
        # relative path — resolve against base_url
        return urllib.parse.urljoin(base_url, href)

    def _inline_css_urls(css: str, base_url: str) -> str:
        """
        Within *css* text, replace all url(...) references to fonts/images
        with base64 data URIs so the stylesheet is fully self-contained.
        """
        def _replace_url(m: re.Match) -> str:
            # group(1) = the URL value (with or without quotes/fragment)
            raw     = m.group(1).strip("'\" ")
            no_frag = raw.split("#")[0]   # strip SVG fragment for fetching
            abs_url = _resolve(no_frag, base_url)
            ext     = os.path.splitext(abs_url.split("?")[0]).lower() if "." in abs_url else ""
            # Only replace font / image assets; leave other url() as-is
            if not any(abs_url.endswith(e) or ("?" in abs_url and abs_url.split("?")[0].endswith(e))
                       for e in _MIME_MAP):
                return m.group(0)
            data_uri = _fetch_binary_data_uri(abs_url)
            return f"url('{data_uri}')"

        return re.sub(r"""url\(\s*(['"]?)([^)'"]+)\1\s*\)""",
                      lambda m: _replace_url(type("M", (), {
                          "group": lambda self, i: [m.group(0), m.group(1), m.group(2)][i]
                      })()),
                      css)

    # Simpler direct replacement using a cleaner regex:
    def _process_css(css: str, base_url: str) -> str:
        def repl(m: re.Match) -> str:
            quote = m.group(1)  # '' or '"' or empty
            href  = m.group(2)
            no_frag = href.split("#")[0]
            abs_url = _resolve(no_frag, base_url)
            # Only inline known font/image types
            ext = os.path.splitext(abs_url.split("?")[0])[1].lower()
            if ext not in _MIME_MAP:
                return m.group(0)
            data_uri = _fetch_binary_data_uri(abs_url)
            return f"url('{data_uri}')"
        return re.sub(r"url\(\s*(['\"]?)([^)'\"\s]+)\1\s*\)", repl, css)

    # ── Inline <script src="https://..."></script> ───────────────────────────
    html = re.sub(
        r'<script\b([^>]*?)\bsrc="(https?://[^"]+)"([^>]*)>\s*</script>',
        lambda m: f"<script>{_fetch_text(m.group(2))}</script>",
        html,
    )

    # ── Inline <link rel="stylesheet" href="https://..."> ────────────────────
    def _inline_css_tag(m: re.Match) -> str:
        full_tag, href = m.group(0), m.group(1)
        if "stylesheet" not in full_tag:
            return full_tag
        css = _fetch_text(href)
        css = _process_css(css, href)   # embed fonts/images referenced inside
        return f"<style>{css}</style>"

    html = re.sub(
        r'<link\b[^>]*\bhref="(https?://[^"]+)"[^>]*/?>',
        _inline_css_tag,
        html,
    )

    return html

    return html


def _sim_to_iso(t_s: float) -> str:
    """Convert a simulation timestamp (seconds) to an ISO-8601 string."""
    return (_SIM_EPOCH + timedelta(seconds=t_s)).strftime("%Y-%m-%dT%H:%M:%S")


def _popup_html(device: str, pos: MobilityPosition) -> str:
    """Build an HTML popup table with all stats for a position snapshot."""
    rows = [
        f"<tr><td><b>device</b></td><td>{device}</td></tr>",
        f"<tr><td><b>t (s)</b></td><td>{pos.timestamp:.3f}</td></tr>",
        f"<tr><td><b>lat</b></td><td>{pos.latitude:.7f}</td></tr>",
        f"<tr><td><b>lon</b></td><td>{pos.longitude:.7f}</td></tr>",
    ]
    for k, v in sorted(pos.extra.items()):
        rows.append(f"<tr><td><b>{k}</b></td><td>{v:.4g}</td></tr>")
    return (
        '<table style="font-size:12px;border-collapse:collapse">'
        + "".join(rows)
        + "</table>"
    )


# ─────────────────────────────────────────────────────────────────────────── #
# Core map builder
# ─────────────────────────────────────────────────────────────────────────── #

def _interval_to_iso8601(seconds: float) -> str:
    """Format a duration in seconds as an ISO-8601 period string (e.g. 'PT10S')."""
    s = max(1, round(seconds))
    if s < 60:
        return f"PT{s}S"
    m, rem = divmod(s, 60)
    if m < 60:
        return f"PT{m}M{rem}S" if rem else f"PT{m}M"
    h, m2 = divmod(m, 60)
    return f"PT{h}H{m2}M{rem}S" if rem else f"PT{h}H{m2}M"


def _build_folium_map(
    by_device: Dict[str, List[MobilityPosition]],
    title: str = "ENIGMA Mobility",
    interval_s: float = 1.0,
) -> "folium.Map":  # type: ignore[name-defined]
    """
    Build and return a folium.Map from a {device → [positions]} dict.
    Heavy import is deferred so callers that don't need viz don't pay the cost.
    """
    import folium  # type: ignore
    from folium.plugins import TimestampedGeoJson  # type: ignore

    colour_map: Dict[str, str] = {}

    # ── Compute map centre ──────────────────────────────────────────────────
    all_lats, all_lons = [], []
    for positions in by_device.values():
        for p in positions:
            all_lats.append(p.latitude)
            all_lons.append(p.longitude)

    centre_lat = sum(all_lats) / max(len(all_lats), 1)
    centre_lon = sum(all_lons) / max(len(all_lons), 1)

    m = folium.Map(
        location=[centre_lat, centre_lon],
        zoom_start=14,
        tiles="OpenStreetMap",
    )

    # Title overlay
    title_html = (
        f'<h3 style="position:fixed;top:10px;left:50%;transform:translateX(-50%);'
        f'z-index:1000;background:rgba(255,255,255,0.85);padding:6px 14px;'
        f'border-radius:6px;font-family:sans-serif">{title}</h3>'
    )
    m.get_root().html.add_child(folium.Element(title_html))

    # ── Trajectory lines ────────────────────────────────────────────────────
    traj_group = folium.FeatureGroup(name="Trajectories", show=True)
    for device, positions in by_device.items():
        col = _colour(device, colour_map)
        coords = [[p.latitude, p.longitude] for p in positions]
        folium.PolyLine(
            locations=coords,
            color=col,
            weight=3,
            opacity=0.7,
            tooltip=device,
        ).add_to(traj_group)
    traj_group.add_to(m)

    # ── Start / End markers ─────────────────────────────────────────────────
    markers_group = folium.FeatureGroup(name="Start / End markers", show=True)
    for device, positions in by_device.items():
        col = _colour(device, colour_map)
        for tag, pos, sym in [("start", positions[0], "&#9654;"),
                               ("end",  positions[-1], "&#9632;")]:
            folium.Marker(
                location=[pos.latitude, pos.longitude],
                icon=folium.DivIcon(
                    html=(
                        f'<div style="color:{col};font-size:18px;'
                        f'text-shadow:1px 1px 2px #000">{sym}</div>'
                    ),
                    icon_size=(22, 22),
                    icon_anchor=(11, 11),
                ),
                popup=folium.Popup(
                    f"<b>{device}</b> ({tag})<br>{_popup_html(device, pos)}",
                    max_width=300,
                ),
                tooltip=f"{device} – {tag}",
            ).add_to(markers_group)
    markers_group.add_to(m)

    # ── Animated TimestampedGeoJson layer ────────────────────────────────────
    features = []
    for device, positions in by_device.items():
        col = _colour(device, colour_map)
        for pos in positions:
            features.append({
                "type": "Feature",
                "geometry": {
                    "type": "Point",
                    "coordinates": [pos.longitude, pos.latitude],
                },
                "properties": {
                    "time": _sim_to_iso(pos.timestamp),
                    "popup": _popup_html(device, pos),
                    "icon": "circle",
                    "iconstyle": {
                        "fillColor": col,
                        "fillOpacity": 0.9,
                        "stroke": True,
                        "color": "#333",
                        "weight": 1,
                        "radius": 7,
                    },
                    "style": {"weight": 0},
                },
            })

    if features:
        period_str   = _interval_to_iso8601(interval_s)
        duration_str = _interval_to_iso8601(interval_s * 1.5)  # show dot until next step
        TimestampedGeoJson(
            data={"type": "FeatureCollection", "features": features},
            period=period_str,
            add_last_point=True,
            auto_play=False,
            loop=False,
            max_speed=5,
            loop_button=True,
            date_options="HH:mm:ss",
            time_slider_drag_update=True,
            duration=duration_str,
        ).add_to(m)

    # ── Colour legend ────────────────────────────────────────────────────────
    legend_items = "".join(
        f'<div><span style="background:{col};display:inline-block;'
        f'width:14px;height:14px;border-radius:50%;margin-right:6px;'
        f'vertical-align:middle"></span>{dev}</div>'
        for dev, col in colour_map.items()
    )
    legend_html = (
        '<div style="position:fixed;bottom:30px;left:20px;z-index:1000;'
        'background:rgba(255,255,255,0.92);padding:10px 14px;border-radius:8px;'
        'box-shadow:2px 2px 6px rgba(0,0,0,0.3);font-family:sans-serif;font-size:13px">'
        f'<b>Devices</b><br>{legend_items}</div>'
    )
    m.get_root().html.add_child(folium.Element(legend_html))

    folium.LayerControl().add_to(m)

    # Auto-fit bounds
    if all_lats and all_lons:
        m.fit_bounds(
            [[min(all_lats), min(all_lons)], [max(all_lats), max(all_lons)]],
            padding=(30, 30),
        )

    return m


# ─────────────────────────────────────────────────────────────────────────── #
# Public API
# ─────────────────────────────────────────────────────────────────────────── #

class MobilityVisualizer:
    """
    Manages live (in-simulation) and post-simulation map visualization.

    Parameters
    ----------
    title : str
        Map title shown in the browser.
    live_path : str
        Path where the live HTML file is written.
    update_interval_s : float
        How often (real-time seconds) the live HTML is regenerated.
    push_rate_limit : float
        Minimum real-time seconds between two ``push()`` calls per device
        that actually enqueue an update (throttle for busy actors).
    """

    def __init__(
        self,
        title: str = "ENIGMA Mobility – Live",
        live_path: str = "/tmp/enigma_mobility_live.html",
        update_interval_s: float = 3.0,
        push_rate_limit: float = 0.2,
        offline: bool = False,
    ) -> None:
        self.title              = title
        self._live_path         = os.path.abspath(live_path)
        self._update_interval   = update_interval_s
        self._push_rate_limit   = push_rate_limit
        self._offline           = offline

        self._lock              = threading.Lock()
        self._buffer: Dict[str, List[MobilityPosition]] = {}
        self._last_push: Dict[str, float] = {}
        # Queue used to send commands to the Playwright thread:
        #   str  → navigate to that URL
        #   None → close browser
        self._playwright_queue: queue.Queue = queue.Queue()

        self._running           = False
        self._thread: Optional[threading.Thread] = None

    # ------------------------------------------------------------------ #
    # Live interface                                                        #
    # ------------------------------------------------------------------ #

    def start(self) -> None:
        """Start the background Playwright browser and refresh thread."""
        self._running = True
        self._write_placeholder()
        self._thread = threading.Thread(
            target=self._refresh_loop,
            daemon=False,
        )
        self._thread.start()
        print(f"[MobilityVisualizer] Live map: file://{self._live_path}")

    def push(self, device: str, pos: MobilityPosition) -> None:
        """
        Add a position snapshot to the live buffer (called from SimGrid actors).
        Rate-limited per device to avoid excessive regenerations.
        """
        now = time.monotonic()
        if now - self._last_push.get(device, 0.0) < self._push_rate_limit:
            return
        self._last_push[device] = now
        with self._lock:
            self._buffer.setdefault(device, []).append(pos)

    def stop(self) -> None:
        """Stop the refresh loop.  The Playwright browser stays open."""
        self._running = False
        # The thread writes the final map and reloads the browser internally;
        # don't join here so the browser remains visible.

    def show_final(self, path: str) -> None:
        """Navigate the live Playwright browser tab to a finished HTML map."""
        self._playwright_queue.put(f"file://{os.path.abspath(path)}")

    def close_browser(self) -> None:
        """
        Close the Playwright browser window.
        Call this explicitly when the application is about to exit.
        """
        self._playwright_queue.put(None)
        if self._thread is not None:
            self._thread.join(timeout=10)

    def wait_for_close(self) -> None:
        """
        Block until the browser window is closed (either by the user clicking
        the X button or by calling ``close_browser()``).
        Ctrl+C triggers a clean close.
        """
        if self._thread is None:
            return
        try:
            while self._thread.is_alive():
                self._thread.join(timeout=0.5)
        except KeyboardInterrupt:
            self.close_browser()

    # ------------------------------------------------------------------ #
    # Post-sim / replay                                                    #
    # ------------------------------------------------------------------ #

    def replay_from_recorder(
        self,
        recorder: MobilityRecorder,
        save_path: Optional[str] = None,
        open_browser: bool = True,
        interval_s: Optional[float] = None,
        offline: Optional[bool] = None,
    ) -> str:
        """
        Build a full interactive map from a MobilityRecorder and save it.
        Returns the absolute path of the saved HTML file.
        """
        path = save_path or self._live_path.replace("_live.html", "_replay.html")
        use_offline = self._offline if offline is None else offline
        MobilityVisualizer.save_interactive_map(
            recorder, path,
            title=self.title.replace("Live", "Replay"),
            interval_s=interval_s,
            offline=use_offline,
        )
        if open_browser:
            _open_browser(f"file://{os.path.abspath(path)}")
        return path

    def replay_from_json(
        self,
        json_path: str,
        save_path: Optional[str] = None,
        open_browser: bool = True,
    ) -> str:
        """Load snapshots from a JSON file and build an interactive map."""
        rec = MobilityVisualizer._recorder_from_json(json_path)
        return self.replay_from_recorder(rec, save_path, open_browser)

    def replay_from_csv(
        self,
        csv_path: str,
        save_path: Optional[str] = None,
        open_browser: bool = True,
    ) -> str:
        """Load snapshots from a CSV file and build an interactive map."""
        rec = MobilityVisualizer._recorder_from_csv(csv_path)
        return self.replay_from_recorder(rec, save_path, open_browser)

    # ------------------------------------------------------------------ #
    # Static helper                                                      #
    # ------------------------------------------------------------------ #

    @staticmethod
    def save_interactive_map(
        recorder: MobilityRecorder,
        filepath: str,
        title: str = "ENIGMA Mobility",
        interval_s: Optional[float] = None,
        offline: bool = False,
    ) -> None:
        """
        Generate a self-contained interactive HTML map from *recorder* and
        save it to *filepath*.

        Parameters
        ----------
        recorder : MobilityRecorder
        filepath : str
            Output ``*.html`` path.
        title : str
        interval_s : float, optional
            Snapshot interval in simulation seconds.  Controls the time-slider
            step size.  If None, auto-inferred from the recorder's timestamps.
        offline : bool
            If True, download all CDN JS/CSS resources and embed them inline
            so the map works without internet access (e.g. opened as
            ``file://``).  Map tiles still require internet at view-time.
            If False (default), the HTML references CDN URLs directly;
            a local HTTP server is needed to avoid browser ``file://`` CORS
            restrictions (run: ``python3 -m http.server 8765``).
        """
        os.makedirs(os.path.dirname(os.path.abspath(filepath)) or ".", exist_ok=True)
        by_dev = recorder.by_device()
        if not by_dev:
            print("[MobilityVisualizer] No data to visualize.")
            return

        step = interval_s if interval_s is not None else recorder.infer_interval_s()
        m = _build_folium_map(by_dev, title=title, interval_s=step)

        # Save to HTML; offline mode: read back and embed CDN resources inline
        m.save(filepath)
        if offline:
            print("[MobilityVisualizer] offline mode: downloading CDN resources…")
            with open(filepath, encoding="utf-8") as f:
                html = f.read()
            html = _embed_cdn_resources(html)
            with open(filepath, "w", encoding="utf-8") as f:
                f.write(html)

        mode = "offline" if offline else "online"
        print(f"[MobilityVisualizer] Interactive map ({mode}) saved → {filepath}")

    # ------------------------------------------------------------------ #
    # Internal helpers                                                   #
    # ------------------------------------------------------------------ #

    def _write_placeholder(self) -> None:
        html = (
            "<!DOCTYPE html><html><head>"
            "<title>ENIGMA Mobility – loading…</title></head>"
            "<body style='font-family:sans-serif;padding:40px'>"
            "<h2>&#9203; Simulation starting – map will appear shortly…</h2>"
            "<p>This page is controlled by Playwright.</p>"
            "</body></html>"
        )
        os.makedirs(os.path.dirname(self._live_path) or ".", exist_ok=True)
        with open(self._live_path, "w") as f:
            f.write(html)

    def _write_live_map(self) -> None:
        with self._lock:
            by_dev = {d: list(ps) for d, ps in self._buffer.items()}
        if not by_dev:
            return
        try:
            import folium  # type: ignore
            # Infer interval from the buffered live data
            step = 1.0
            for positions in by_dev.values():
                if len(positions) >= 2:
                    gaps = sorted(
                        positions[i + 1].timestamp - positions[i].timestamp
                        for i in range(len(positions) - 1)
                        if positions[i + 1].timestamp > positions[i].timestamp
                    )
                    if gaps:
                        step = gaps[len(gaps) // 2]
                        break
            m = _build_folium_map(by_dev, title=self.title, interval_s=step)
            tmp = self._live_path + ".tmp"
            m.save(tmp)
            with open(tmp, encoding="utf-8") as f:
                content = f.read()
            os.remove(tmp)
            if self._offline:
                content = _embed_cdn_resources(content)
            with open(self._live_path, "w") as f:
                f.write(content)
        except Exception as exc:
            print(f"[MobilityVisualizer] warning: could not write live map: {exc}")

    def _refresh_loop(self) -> None:
        self._refresh_loop_playwright()

    def _refresh_loop_playwright(self) -> None:
        """
        Background thread that:
        1. Launches a visible Playwright/Chromium browser at the live HTML file.
        2. Rewrites the file every *update_interval_s* real-time seconds and
           calls ``page.reload()`` – no meta-refresh needed, no caching issues,
           works with any ``file://`` path regardless of Snap AppArmor rules.
        3. After the simulation ends (``_running`` becomes False) does one final
           write+reload, then waits for ``show_final()`` / ``close_browser()``
           commands from the main thread.
        """
        try:
            from playwright.sync_api import sync_playwright  # type: ignore
        except ImportError:
            print(
                "[MobilityVisualizer] ERROR: playwright not installed.\n"
                "  pip install playwright && playwright install chromium"
            )
            return

        live_url = f"file://{self._live_path}"
        try:
            with sync_playwright() as pw:
                browser = pw.chromium.launch(
                    headless=False,
                    args=[
                        "--allow-file-access-from-files",
                        "--disable-web-security",
                        "--no-sandbox",
                        "--start-maximized",
                    ],
                )
                # Event set when the browser window is closed by the user or
                # the browser process exits for any reason.
                _closed = threading.Event()
                browser.on("disconnected", lambda _b: _closed.set())

                try:
                    context = browser.new_context(
                        # Allow all file:// origins so folium CDN-less maps render.
                        bypass_csp=True,
                        no_viewport=True,   # let the OS/window manager set the size
                    )
                    page = context.new_page()
                    # "close" fires immediately when the user closes the
                    # window/tab – more reliable than the browser-level
                    # "disconnected" event in the sync API.
                    page.on("close", lambda _p: _closed.set())
                    page.goto(live_url)
                    # Maximize / fullscreen the window
                    try:
                        page.evaluate("() => document.documentElement.requestFullscreen()")
                    except Exception:
                        pass  # not all platforms support the Fullscreen API via file://
                    print(
                        f"[MobilityVisualizer] Playwright browser opened: {live_url}"
                    )

                    # ---- live refresh loop ------------------------------------ #
                    while self._running and not _closed.is_set():
                        time.sleep(self._update_interval)
                        if _closed.is_set():
                            break
                        self._write_live_map()
                        if not _closed.is_set():
                            try:
                                page.reload()
                            except Exception:
                                _closed.set()
                                break

                    # ---- simulation ended: final snapshot -------------------- #
                    if not _closed.is_set():
                        self._write_live_map()
                        try:
                            page.reload()
                        except Exception:
                            _closed.set()

                    # ---- wait for show_final() / close_browser() ------------- #
                    # Drain the command queue; None = close,  str = navigate.
                    # We detect browser closure by actively pinging the page
                    # with page.evaluate("1") on every idle tick – this makes a
                    # real CDP round-trip and throws the instant Chromium is gone,
                    # regardless of whether Playwright fires "close"/"disconnected"
                    # events promptly in the sync API.
                    while not _closed.is_set():
                        try:
                            cmd = self._playwright_queue.get(timeout=0.1)
                            if cmd is None:
                                break
                            if isinstance(cmd, str):
                                try:
                                    page.goto(cmd)
                                except Exception:
                                    _closed.set()
                                    break
                        except queue.Empty:
                            # No command – ping the page to detect a closed browser.
                            try:
                                page.evaluate("1")
                            except Exception:
                                _closed.set()
                                break

                    if not _closed.is_set():
                        try:
                            browser.close()
                        except Exception:
                            pass

                except KeyboardInterrupt:
                    # Ctrl+C delivered to this background thread – close quietly.
                    print("[MobilityVisualizer] Playwright thread interrupted; closing browser.")
                    if not _closed.is_set():
                        try:
                            browser.close()
                        except Exception:
                            pass

        except Exception as exc:
            print(f"[MobilityVisualizer] Playwright error: {exc}")

    # ------------------------------------------------------------------ #
    # Load helpers for replay                                              #
    # ------------------------------------------------------------------ #

    @staticmethod
    def _recorder_from_json(path: str) -> MobilityRecorder:
        import json as _json
        rec = MobilityRecorder()
        with open(path) as f:
            data = _json.load(f)
        for row in data:
            row = dict(row)
            device = row.pop("device", "unknown")
            ts  = float(row.pop("timestamp", 0))
            lat = float(row.pop("latitude",  0))
            lon = float(row.pop("longitude", 0))
            extra = {k: float(v) for k, v in row.items()
                     if isinstance(v, (int, float))}
            rec.add(device, MobilityPosition(ts, lat, lon, extra))
        return rec

    @staticmethod
    def _recorder_from_csv(path: str) -> MobilityRecorder:
        import csv as _csv
        rec = MobilityRecorder()
        with open(path, newline="") as f:
            reader = _csv.DictReader(f)
            for row in reader:
                row = dict(row)
                device = row.pop("device", "unknown")
                ts  = float(row.pop("timestamp", 0))
                lat = float(row.pop("latitude",  0))
                lon = float(row.pop("longitude", 0))
                extra: Dict[str, float] = {}
                for k, v in row.items():
                    try:
                        extra[k] = float(v)
                    except (ValueError, TypeError):
                        pass
                rec.add(device, MobilityPosition(ts, lat, lon, extra))
        return rec
