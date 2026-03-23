/**
 * @file MobilityManager.cpp
 * @brief Implementation of MobilityManager.
 */

#include "mobility/MobilityManager.hpp"

#include <xbt/log.h>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <set>
#include <sstream>
#include <stdexcept>

XBT_LOG_NEW_DEFAULT_CATEGORY(mobility_manager, "ENIGMA Mobility Manager");

namespace fs = std::filesystem;

namespace enigma::mobility {

// ------------------------------------------------------------------ //
// helpers
// ------------------------------------------------------------------ //

static std::string quote(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"') out += "\\\"";
        else          out += c;
    }
    return out + "\"";
}

// ------------------------------------------------------------------ //
// Constructors
// ------------------------------------------------------------------ //

std::string MobilityManager::detect_mobility_dir(simgrid::s4u::Engine& engine) const {
    // Walk all zones and look for the "mobility_dir" property
    for (auto* zone : engine.get_all_netzones()) {
        const char* val = zone->get_property("mobility_dir");
        if (val && val[0] != '\0') {
            XBT_INFO("Found mobility_dir property in zone '%s': %s",
                     zone->get_cname(), val);
            return std::string(val);
        }
    }
    // Also check host-level properties
    for (auto* host : engine.get_all_hosts()) {
        const char* val = host->get_property("mobility_dir");
        if (val && val[0] != '\0') {
            XBT_INFO("Found mobility_dir property on host '%s': %s",
                     host->get_cname(), val);
            return std::string(val);
        }
    }
    return {};
}

MobilityManager::MobilityManager(simgrid::s4u::Engine& engine) {
    std::string dir = detect_mobility_dir(engine);
    if (dir.empty()) {
        XBT_INFO("No mobility_dir property found – mobility module inactive");
        return;
    }
    coords_dir_ = dir;
    load_directory(dir);
}

MobilityManager::MobilityManager(simgrid::s4u::Engine& engine,
                                   const std::string& coords_dir)
    : coords_dir_(coords_dir) {
    (void)engine;
    load_directory(coords_dir);
}

// ------------------------------------------------------------------ //
// Load directory
// ------------------------------------------------------------------ //

void MobilityManager::load_directory(const std::string& dir_path) {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        XBT_WARN("Mobility dir '%s' does not exist or is not a directory", dir_path.c_str());
        return;
    }

    int loaded = 0;
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".csv") continue;

        try {
            MobilityTrace t(entry.path().string());
            XBT_DEBUG("Loaded trace '%s': %zu waypoints [%.1f – %.1f s]",
                      t.device_name().c_str(), t.size(), t.t_min(), t.t_max());
            traces_.emplace(t.device_name(), std::move(t));
            ++loaded;
        } catch (const std::exception& ex) {
            XBT_WARN("Could not load '%s': %s", entry.path().c_str(), ex.what());
        }
    }
    XBT_INFO("MobilityManager: loaded %d traces from '%s'", loaded, dir_path.c_str());
}

// ------------------------------------------------------------------ //
// Query
// ------------------------------------------------------------------ //

bool MobilityManager::has_trace(const std::string& device_name) const noexcept {
    return traces_.find(device_name) != traces_.end();
}

std::optional<MobilityPosition>
MobilityManager::position_at(const std::string& device_name, double sim_t) const noexcept {
    auto it = traces_.find(device_name);
    if (it == traces_.end()) return std::nullopt;
    return it->second.position_at(sim_t);
}

// ------------------------------------------------------------------ //
// Snapshot recording
// ------------------------------------------------------------------ //

void MobilityManager::record_all(double sim_t) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& [name, trace] : traces_) {
        snapshots_.push_back({name, trace.position_at(sim_t)});
    }
}

void MobilityManager::record(const std::string& device_name, double sim_t) {
    auto it = traces_.find(device_name);
    if (it == traces_.end()) return;
    std::lock_guard<std::mutex> lock(mutex_);
    snapshots_.push_back({device_name, it->second.position_at(sim_t)});
}

// ------------------------------------------------------------------ //
// Periodic actor
// ------------------------------------------------------------------ //

void MobilityManager::start_periodic_actor(simgrid::s4u::Engine& engine,
                                             double interval_s) {
    if (traces_.empty()) return;

    auto* host = engine.get_all_hosts().front();
    XBT_INFO("Starting MobilityRecorder actor on host '%s' (interval=%.1f s)",
             host->get_cname(), interval_s);

    // Determine latest trace timestamp so the actor auto-terminates
    double t_end = 0.0;
    for (const auto& [name, trace] : traces_)
        t_end = std::max(t_end, trace.t_max());
    t_end += interval_s * 2.0;  // small safety buffer

    // Capture by pointer (manager must outlive the simulation)
    MobilityManager* self = this;
    host->add_actor("mobility_recorder",
        [self, interval_s, t_end]() {
            XBT_INFO("MobilityRecorder started (interval=%.1f s, runs until t=%.1f s)",
                     interval_s, t_end);
            while (simgrid::s4u::Engine::get_clock() <= t_end) {
                double t = simgrid::s4u::Engine::get_clock();
                self->record_all(t);
                simgrid::s4u::this_actor::sleep_for(interval_s);
            }
            XBT_INFO("MobilityRecorder finished");
        });
}

// ------------------------------------------------------------------ //
// Export
// ------------------------------------------------------------------ //

void MobilityManager::export_json(const std::string& filename) const {
    std::ofstream f(filename);
    if (!f) throw std::runtime_error("Cannot open '" + filename + "' for writing");

    f << "[\n";
    bool first = true;
    for (const auto& s : snapshots_) {
        if (!first) f << ",\n";
        first = false;
        const auto& p = s.position;
        f << "  {"
          << "\"device\":"    << quote(s.device_name) << ","
          << "\"timestamp\":" << std::fixed << std::setprecision(6) << p.timestamp << ","
          << "\"latitude\":"  << p.latitude  << ","
          << "\"longitude\":" << p.longitude;
        for (const auto& [key, val] : p.extra)
            f << ",\"" << key << "\":" << val;
        f << "}";
    }
    f << "\n]\n";
    XBT_INFO("Exported %zu snapshots to '%s'", snapshots_.size(), filename.c_str());
}

void MobilityManager::export_csv(const std::string& filename) const {
    std::ofstream f(filename);
    if (!f) throw std::runtime_error("Cannot open '" + filename + "' for writing");

    // Collect all extra key names (in insertion/sorted order across all snapshots)
    std::vector<std::string> extra_keys;
    {
        std::set<std::string> seen;
        for (const auto& s : snapshots_)
            for (const auto& [k, _] : s.position.extra)
                if (seen.insert(k).second) extra_keys.push_back(k);
    }

    // Header
    f << "device,timestamp,latitude,longitude";
    for (const auto& k : extra_keys) f << "," << k;
    f << "\n";

    // Rows
    for (const auto& s : snapshots_) {
        const auto& p = s.position;
        f << s.device_name << ","
          << std::fixed << std::setprecision(6)
          << p.timestamp  << "," << p.latitude  << "," << p.longitude;
        for (const auto& k : extra_keys) {
            auto it = p.extra.find(k);
            f << "," << (it != p.extra.end() ? it->second : 0.0);
        }
        f << "\n";
    }
    XBT_INFO("Exported %zu snapshots to '%s'", snapshots_.size(), filename.c_str());
}

void MobilityManager::export_traces_json(const std::string& filename) const {
    std::ofstream f(filename);
    if (!f) throw std::runtime_error("Cannot open '" + filename + "' for writing");

    f << "{\n";
    bool first_trace = true;
    for (const auto& [name, trace] : traces_) {
        if (!first_trace) f << ",\n";
        first_trace = false;
        f << "  " << quote(name) << ": [\n";
        bool first_wp = true;
        for (const auto& p : trace.waypoints()) {
            if (!first_wp) f << ",\n";
            first_wp = false;
            f << "    {"
              << "\"timestamp\":" << std::fixed << std::setprecision(6) << p.timestamp << ","
              << "\"latitude\":"  << p.latitude  << ","
              << "\"longitude\":" << p.longitude;
            for (const auto& [key, val] : p.extra)
                f << ",\"" << key << "\":" << val;
            f << "}";
        }
        f << "\n  ]";
    }
    f << "\n}\n";
    XBT_INFO("Exported raw traces for %zu devices to '%s'",
             traces_.size(), filename.c_str());
}

} // namespace enigma::mobility
