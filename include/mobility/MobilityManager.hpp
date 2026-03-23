#pragma once
/**
 * @file MobilityManager.hpp
 * @brief Manages mobility traces for all devices in the simulation.
 *
 * # Usage (C++ side)
 *
 * ## 1. Specify the coords directory in the platform XML
 *
 * Add a property to any zone (or the root zone) of the platform:
 * @code{.xml}
 *   <zone id="root" routing="Full">
 *     <prop id="mobility_dir" value="platforms/coords/"/>
 *     ...
 *   </zone>
 * @endcode
 * The directory is relative to the working directory when the simulator runs.
 *
 * ## 2. Create the manager after loading the platform
 * @code
 *   simgrid::s4u::Engine* e = ...;
 *   enigma::mobility::MobilityManager mob(*e);
 *   // or with an explicit path:
 *   enigma::mobility::MobilityManager mob(*e, "my_coords/");
 * @endcode
 *
 * ## 3. Query positions inside actors
 * @code
 *   auto pos = mob.position_at("edge_cluster_0_node_1",
 *                               simgrid::s4u::Engine::get_clock());
 *   if (pos) XBT_INFO("lat=%.6f lon=%.6f", pos->latitude, pos->longitude);
 * @endcode
 *
 * ## 4. Optionally: launch the periodic snapshot actor
 * @code
 *   mob.start_periodic_actor(*e, 1.0);  // snapshot every 1 sim-second
 * @endcode
 *
 * ## 5. Export after e.run()
 * @code
 *   mob.export_json("mobility_output.json");
 *   mob.export_csv("mobility_output.csv");
 * @endcode
 */

#include "MobilityPosition.hpp"
#include "MobilityTrace.hpp"

#include <simgrid/s4u.hpp>

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace enigma::mobility {

/// A recorded snapshot: device name + interpolated position.
struct Snapshot {
    std::string    device_name;
    MobilityPosition position;
};

class MobilityManager {
public:
    /**
     * @brief Construct, auto-detecting the coords directory.
     *
     * Reads the `mobility_dir` property from the first zone of @p engine's
     * platform that exposes it.  Loads all *.csv files found there.
     * If the property is absent, no traces are loaded (the manager is a no-op).
     */
    explicit MobilityManager(simgrid::s4u::Engine& engine);

    /**
     * @brief Construct with an explicit directory path.
     * Loads all *.csv files inside @p coords_dir.
     */
    MobilityManager(simgrid::s4u::Engine& engine, const std::string& coords_dir);

    // ------------------------------------------------------------------ //
    // Query
    // ------------------------------------------------------------------ //

    /// Number of device traces loaded.
    std::size_t trace_count() const noexcept { return traces_.size(); }

    /// True if a trace is available for @p device_name.
    bool has_trace(const std::string& device_name) const noexcept;

    /**
     * @brief Interpolated position for @p device_name at @p sim_t.
     * Returns std::nullopt if no trace is loaded for that device.
     */
    std::optional<MobilityPosition> position_at(const std::string& device_name,
                                                  double sim_t) const noexcept;

    // ------------------------------------------------------------------ //
    // Snapshot recording
    // ------------------------------------------------------------------ //

    /**
     * @brief Record a snapshot of ALL devices at @p sim_t.
     * Thread-safe; can be called from any actor.
     */
    void record_all(double sim_t);

    /**
     * @brief Record a snapshot for ONE device at @p sim_t.
     * Thread-safe.
     */
    void record(const std::string& device_name, double sim_t);

    /// All recorded snapshots (ordered by time of record() call).
    const std::vector<Snapshot>& snapshots() const noexcept { return snapshots_; }

    // ------------------------------------------------------------------ //
    // Periodic actor
    // ------------------------------------------------------------------ //

    /**
     * @brief Deploy a SimGrid actor that calls record_all() every
     *        @p interval_s simulated seconds for the duration of the sim.
     *
     * Must be called before e.run().  The actor runs on the first host of
     * the platform (it performs no networking, just bookkeeping).
     */
    void start_periodic_actor(simgrid::s4u::Engine& engine, double interval_s = 1.0);

    // ------------------------------------------------------------------ //
    // Export
    // ------------------------------------------------------------------ //

    /**
     * @brief Export all recorded snapshots to a JSON file.
     * Format: array of {device, timestamp, lat, lon, alt, speed, heading, accuracy}
     */
    void export_json(const std::string& filename) const;

    /**
     * @brief Export all recorded snapshots to a CSV file.
     */
    void export_csv(const std::string& filename) const;

    /**
     * @brief Export full raw traces (not just snapshots) to JSON.
     * Useful when no periodic actor was used.
     */
    void export_traces_json(const std::string& filename) const;

private:
    void load_directory(const std::string& dir_path);
    std::string detect_mobility_dir(simgrid::s4u::Engine& engine) const;

    std::map<std::string, MobilityTrace> traces_;  ///< keyed by device_name
    std::vector<Snapshot>               snapshots_;
    mutable std::mutex                  mutex_;
    std::string                         coords_dir_;
};

} // namespace enigma::mobility
