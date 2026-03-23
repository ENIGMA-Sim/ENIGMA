/**
 * @file MobilityTrace.cpp
 * @brief Implementation of MobilityTrace.
 */

#include "mobility/MobilityTrace.hpp"

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace enigma::mobility {

// ------------------------------------------------------------------ //
// Helpers
// ------------------------------------------------------------------ //

static double lerp_d(double a, double b, double t) noexcept {
    return a + (b - a) * t;
}

/// Lowercase + trim a string token.
static std::string norm(std::string s) {
    // trim leading/trailing whitespace
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    s = s.substr(first, s.find_last_not_of(" \t\r\n") - first + 1);
    // to lower
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

// ------------------------------------------------------------------ //
// MobilityTrace
// ------------------------------------------------------------------ //

std::string MobilityTrace::stem(const std::string& path) {
    auto slash = path.find_last_of("/\\");
    std::string filename = (slash == std::string::npos) ? path : path.substr(slash + 1);
    auto dot = filename.rfind('.');
    return (dot == std::string::npos) ? filename : filename.substr(0, dot);
}

MobilityTrace::MobilityTrace(const std::string& csv_path)
    : device_name_(stem(csv_path)), csv_path_(csv_path) {

    std::ifstream f(csv_path);
    if (!f.is_open())
        throw std::runtime_error("MobilityTrace: cannot open '" + csv_path + "'");

    std::string line;
    if (!std::getline(f, line))
        throw std::runtime_error("MobilityTrace: empty file '" + csv_path + "'");

    // ------------------------------------------------------------------
    // Parse header – find index for the three required columns and
    // collect "extra" (canonical-name, raw-index) pairs for all others.
    // ------------------------------------------------------------------
    // Accepted aliases
    static const std::set<std::string> TS_ALIASES  = {"timestamp","time","t","ts","time_s","sim_time"};
    static const std::set<std::string> LAT_ALIASES = {"latitude","lat","lat_deg"};
    static const std::set<std::string> LON_ALIASES = {"longitude","lon","lng","lon_deg","long"};

    int c_ts = -1, c_lat = -1, c_lon = -1;
    // extra: (canonical_name, column_index)
    std::vector<std::pair<std::string, int>> extra_cols;

    {
        std::istringstream ss(line);
        std::string tok;
        int idx = 0;
        while (std::getline(ss, tok, ',')) {
            std::string n = norm(tok);
            if (c_ts  == -1 && TS_ALIASES .count(n)) { c_ts  = idx; }
            else if (c_lat == -1 && LAT_ALIASES.count(n)) { c_lat = idx; }
            else if (c_lon == -1 && LON_ALIASES.count(n)) { c_lon = idx; }
            else { extra_cols.push_back({n, idx}); }
            ++idx;
        }
    }

    if (c_ts < 0 || c_lat < 0 || c_lon < 0)
        throw std::runtime_error(
            "MobilityTrace: CSV '" + csv_path +
            "' must have timestamp, latitude and longitude columns "
            "(checked aliases: time/t/ts, lat, lon/lng)");

    // ------------------------------------------------------------------
    // Parse data rows
    // ------------------------------------------------------------------
    while (std::getline(f, line)) {
        if (line.empty() || line[0] == '#') continue;

        // Split into token vector
        std::vector<std::string> fields;
        {
            std::istringstream ss(line);
            std::string tok;
            while (std::getline(ss, tok, ','))
                fields.push_back(tok);
        }

        int max_req = std::max({c_ts, c_lat, c_lon});
        if (static_cast<int>(fields.size()) <= max_req) continue; // malformed

        try {
            MobilityPosition p;
            p.timestamp  = std::stod(fields[c_ts]);
            p.latitude   = std::stod(fields[c_lat]);
            p.longitude  = std::stod(fields[c_lon]);
            for (const auto& [name, cidx] : extra_cols) {
                if (cidx < static_cast<int>(fields.size())) {
                    const std::string& val = fields[cidx];
                    if (!val.empty()) {
                        try { p.extra[name] = std::stod(val); }
                        catch (...) { /* non-numeric extra column – skip this field */ }
                    }
                }
            }
            waypoints_.push_back(std::move(p));
        } catch (...) {
            continue; // skip malformed rows (bad timestamp/lat/lon)
        }
    }

    if (waypoints_.empty())
        throw std::runtime_error("MobilityTrace: no valid rows in '" + csv_path + "'");

    std::sort(waypoints_.begin(), waypoints_.end(),
              [](const MobilityPosition& a, const MobilityPosition& b) {
                  return a.timestamp < b.timestamp;
              });
}

MobilityPosition MobilityTrace::lerp(const MobilityPosition& a,
                                      const MobilityPosition& b,
                                      double t) noexcept {
    MobilityPosition p;
    p.timestamp  = lerp_d(a.timestamp, b.timestamp, t);
    p.latitude   = lerp_d(a.latitude,  b.latitude,  t);
    p.longitude  = lerp_d(a.longitude, b.longitude, t);

    // Interpolate all extra keys that appear in either waypoint
    for (const auto& [key, va] : a.extra) {
        double vb = 0.0;
        auto it = b.extra.find(key);
        if (it != b.extra.end()) vb = it->second;
        p.extra[key] = lerp_d(va, vb, t);
    }
    for (const auto& [key, vb] : b.extra) {
        if (p.extra.find(key) == p.extra.end())
            p.extra[key] = lerp_d(0.0, vb, t);
    }
    return p;
}

MobilityPosition MobilityTrace::position_at(double sim_t) const noexcept {
    if (waypoints_.empty()) return {};
    if (sim_t <= waypoints_.front().timestamp) return waypoints_.front();
    if (sim_t >= waypoints_.back().timestamp)  return waypoints_.back();

    auto it = std::upper_bound(waypoints_.begin(), waypoints_.end(), sim_t,
                               [](double t, const MobilityPosition& p) {
                                   return t < p.timestamp;
                               });
    const MobilityPosition& b = *it;
    const MobilityPosition& a = *(it - 1);
    double frac = (sim_t - a.timestamp) / (b.timestamp - a.timestamp);
    MobilityPosition p = lerp(a, b, frac);
    p.timestamp = sim_t;
    return p;
}

} // namespace enigma::mobility
