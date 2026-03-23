#pragma once
/**
 * @file MobilityPosition.hpp
 * @brief Single geo-position snapshot for a device.
 *
 * ENIGMA Mobility Module
 * ----------------------
 * Only ``timestamp``, ``latitude`` and ``longitude`` are required (and are
 * first-class struct members).  **All other CSV columns – whatever their
 * name – are stored in** ``extra`` as a ``std::map<std::string, double>``
 * and are serialised/interpolated automatically.  There is no assumed schema
 * beyond the three mandatory fields.
 */

#include <cmath>
#include <map>
#include <string>

namespace enigma::mobility {

/// Earth radius in metres (WGS-84 mean)
constexpr double EARTH_RADIUS_M = 6'371'000.0;

/// One position snapshot (one CSV row).
struct MobilityPosition {
    double timestamp  = 0.0;   ///< Simulation or wall-clock time (s)
    double latitude   = 0.0;   ///< Decimal degrees, WGS-84
    double longitude  = 0.0;   ///< Decimal degrees, WGS-84

    /// All other CSV columns keyed by their lower-cased column name.
    std::map<std::string, double> extra;

    /// Haversine distance to *other* in metres.
    double distance_to(const MobilityPosition& other) const noexcept {
        const double phi1 = latitude  * M_PI / 180.0;
        const double phi2 = other.latitude  * M_PI / 180.0;
        const double dphi = (other.latitude  - latitude)  * M_PI / 180.0;
        const double dlam = (other.longitude - longitude) * M_PI / 180.0;
        const double a = std::sin(dphi / 2) * std::sin(dphi / 2) +
                         std::cos(phi1) * std::cos(phi2) *
                         std::sin(dlam / 2) * std::sin(dlam / 2);
        return EARTH_RADIUS_M * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
    }
};

} // namespace enigma::mobility
