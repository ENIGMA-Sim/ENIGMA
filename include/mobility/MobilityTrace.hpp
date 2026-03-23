#pragma once
/**
 * @file MobilityTrace.hpp
 * @brief Load and interpolate one device's GPS trace from a CSV file.
 *
 * CSV format (header row mandatory):
 *   timestamp, latitude, longitude [, <any extra column>...]
 *
 * Only ``timestamp`` (aliases: time, t, ts),
 * ``latitude`` (alias: lat), and ``longitude`` (aliases: lon, lng)
 * are required.  All other columns are stored in
 * ``MobilityPosition::extra`` keyed by their lower-cased column name.
 * There is no assumed schema beyond the three mandatory fields.
 *
 * The trace is sorted by timestamp on load; queries outside the range are
 * clamped to the first / last entry.
 *
 * Example:
 * @code
 *   MobilityTrace t("coords/device1.csv");
 *   auto pos = t.position_at(3.5);   // interpolated at t=3.5 s
 *   double alt = pos.extra.count("altitude") ? pos.extra.at("altitude") : 0.0;
 * @endcode
 */

#include "MobilityPosition.hpp"
#include <optional>
#include <string>
#include <vector>

namespace enigma::mobility {

class MobilityTrace {
public:
    /// Load trace from CSV file.  Throws std::runtime_error on I/O failure.
    explicit MobilityTrace(const std::string& csv_path);

    /// Device/host name (derived from filename stem).
    const std::string& device_name() const noexcept { return device_name_; }

    /// Path to the source CSV file.
    const std::string& csv_path() const noexcept { return csv_path_; }

    /// Number of waypoints loaded.
    std::size_t size() const noexcept { return waypoints_.size(); }

    /// Earliest timestamp in the trace (seconds).
    double t_min() const noexcept { return waypoints_.empty() ? 0.0 : waypoints_.front().timestamp; }

    /// Latest  timestamp in the trace (seconds).
    double t_max() const noexcept { return waypoints_.empty() ? 0.0 : waypoints_.back().timestamp; }

    /// All raw waypoints (sorted by timestamp).
    const std::vector<MobilityPosition>& waypoints() const noexcept { return waypoints_; }

    /**
     * @brief Linear interpolation / extrapolation at simulation time @p sim_t.
     *
     * - Before t_min: returns the first waypoint.
     * - After  t_max: returns the last  waypoint.
     * - Otherwise:    linearly interpolates between the two surrounding entries.
     *
     * @return Interpolated MobilityPosition with timestamp == sim_t.
     */
    MobilityPosition position_at(double sim_t) const noexcept;

private:
    std::string device_name_;
    std::string csv_path_;
    std::vector<MobilityPosition> waypoints_;

    static std::string stem(const std::string& path);
    static MobilityPosition lerp(const MobilityPosition& a,
                                  const MobilityPosition& b,
                                  double t) noexcept;
};

} // namespace enigma::mobility
