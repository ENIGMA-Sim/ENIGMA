/**
 * @file mobility_test.cpp
 * @brief ENIGMA Mobility module demonstration.
 *
 * Shows how to:
 *  - Declare the coords directory as a zone property in the XML
 *  - Load all traces via MobilityManager
 *  - Query device positions inside SimGrid actors
 *  - Dump snapshots to JSON + CSV for post-simulation visualisation
 *
 * Usage
 * -----
 *   ./mobility_test_app <platform_file.xml> [output_prefix]
 *
 * The platform XML must contain either a zone- or host-level property:
 *   <prop id="mobility_dir" value="platforms/coords/"/>
 *
 * Alternatively pass --mobility-dir <dir> as argument (see below).
 */

#include <simgrid/s4u.hpp>
#include <iostream>

#include "mobility/MobilityManager.hpp"
#include "mobility/MobilityPosition.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(mobility_test, "ENIGMA Mobility Test");

namespace sg4 = simgrid::s4u;
using namespace enigma::mobility;

// -------------------------------------------------------------------------- //
// Actor: periodically logs its own position to the SimGrid log              //
// -------------------------------------------------------------------------- //

class MobileActor {
    std::string    host_name_;
    MobilityManager* mob_;
    double         interval_;
    int            iterations_;

public:
    MobileActor(std::string name, MobilityManager* mob,
                double interval = 2.0, int iterations = 10)
        : host_name_(std::move(name))
        , mob_(mob)
        , interval_(interval)
        , iterations_(iterations) {}

    void operator()() const {
        for (int i = 0; i < iterations_; ++i) {
            double t = sg4::Engine::get_clock();
            auto pos = mob_->position_at(host_name_, t);
            if (pos) {
                double spd = pos->extra.count("speed")   ? pos->extra.at("speed")   : 0.0;
                double hdg = pos->extra.count("heading") ? pos->extra.at("heading") : 0.0;
                XBT_INFO("[%.2f s] %s → lat=%.6f lon=%.6f  spd=%.1f m/s  hdg=%.1f°",
                         t, host_name_.c_str(),
                         pos->latitude, pos->longitude, spd, hdg);
            } else {
                XBT_INFO("[%.2f s] %s → no mobility trace available", t, host_name_.c_str());
            }
            // Simulate some local work
            sg4::this_actor::execute(5e8);
            sg4::this_actor::sleep_for(interval_);
        }
        XBT_INFO("%s: actor finished", host_name_.c_str());
    }
};

// -------------------------------------------------------------------------- //
// main                                                                       //
// -------------------------------------------------------------------------- //

int main(int argc, char** argv) {
    sg4::Engine e(&argc, argv);

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <platform.xml> [output_prefix] [--mobility-dir <dir>]\n";
        return 1;
    }

    const std::string platform_file = argv[1];
    std::string output_prefix = "mobility_output";
    std::string mobility_dir;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mobility-dir" && i + 1 < argc) {
            mobility_dir = argv[++i];
        } else if (arg[0] != '-') {
            output_prefix = arg;
        }
    }

    e.load_platform(platform_file);

    // ------------------------------------------------------------------ //
    // Create MobilityManager (reads mobility_dir from XML prop or arg)    //
    // ------------------------------------------------------------------ //
    std::unique_ptr<MobilityManager> mob;
    if (!mobility_dir.empty()) {
        mob = std::make_unique<MobilityManager>(e, mobility_dir);
    } else {
        mob = std::make_unique<MobilityManager>(e);
    }

    XBT_INFO("=== Mobility Test Application ===");
    XBT_INFO("Platform file : %s", platform_file.c_str());
    XBT_INFO("Traces loaded : %zu", mob->trace_count());
    XBT_INFO("Output prefix : %s", output_prefix.c_str());

    // ------------------------------------------------------------------ //
    // Start periodic snapshot actor (records positions every 0.5 sim s)   //
    // ------------------------------------------------------------------ //
    mob->start_periodic_actor(e, 0.5);

    // ------------------------------------------------------------------ //
    // Deploy a MobileActor on every host that has a trace                 //
    // ------------------------------------------------------------------ //
    auto hosts = e.get_all_hosts();
    int deployed = 0;
    for (auto* host : hosts) {
        if (mob->has_trace(host->get_cname())) {
            host->add_actor("mobile_actor",
                            MobileActor(host->get_cname(), mob.get(), 1.0, 8));
            XBT_INFO("Deployed mobile actor on '%s'", host->get_cname());
            ++deployed;
        }
    }

    if (deployed == 0) {
        XBT_WARN("No matching traces found for any host. "
                 "Check that CSV filenames match host names.");
    }

    // ------------------------------------------------------------------ //
    // Run simulation                                                       //
    // ------------------------------------------------------------------ //
    e.run();

    XBT_INFO("=== Simulation completed – t=%.3f s ===", sg4::Engine::get_clock());

    // ------------------------------------------------------------------ //
    // Export results                                                       //
    // ------------------------------------------------------------------ //
    const std::string json_out = output_prefix + "_snapshots.json";
    const std::string csv_out  = output_prefix + "_snapshots.csv";
    const std::string raw_json = output_prefix + "_raw_traces.json";

    mob->export_json(json_out);
    mob->export_csv(csv_out);
    mob->export_traces_json(raw_json);

    XBT_INFO("Outputs written:");
    XBT_INFO("  Snapshots JSON : %s  (%zu records)", json_out.c_str(), mob->snapshots().size());
    XBT_INFO("  Snapshots CSV  : %s", csv_out.c_str());
    XBT_INFO("  Raw traces JSON: %s", raw_json.c_str());
    XBT_INFO("Visualise with:");
    XBT_INFO("  python3 src/python/tools/mobility_viewer.py %s", json_out.c_str());

    return 0;
}
