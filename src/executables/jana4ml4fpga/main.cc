//
// Created by xmei on 2/27/23.
//

#include <iostream>

#include <TFile.h>

#include "JANA4ML4FPGA_CLI.h"

using namespace jana;

/// The default plugins
/// Add new default plugin names here and the main() will do JApplication::AddPlugin() for you.
std::vector<std::string> JANA4ML4FPGA_DEFAULT_PLUGINS = {
    "log",
};

int main( int narg, char *argv[]) {

    std::vector<std::string> default_plugins = JANA4ML4FPGA_DEFAULT_PLUGINS;

    auto options = jana::GetCliOptions(narg, argv, false);

    if (HasPrintOnlyCliOptions(options, default_plugins))
        return 0;

    /// take plugins from both default plugins and cli input
    /// @note: do not search for available plugins at various paths
    AddPluginsToOptionParams(options, default_plugins);

    japp = jana::CreateJApplication(options);

    auto exit_code = jana::Execute(japp, options);  /// topology is added inside

    delete japp;
    return exit_code;
}
