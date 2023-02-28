//
// Created by xmei on 2/27/23.
//

#include "PrintUtils.h"
#include "JANA4ML4FPGA_CLI.h"

#include <JANA/CLI/JVersion.h>
#include <JANA/CLI/JBenchmarker.h>
#include <JANA/CLI/JSignalHandler.h>

#include <JANA/Services/JComponentManager.h>

#include <set>
#include <iostream>
#include <string>
#include <filesystem>

#define QUOTE(name) #name
#define STR(macro) QUOTE(macro)

#ifndef JANA4ML4FPGA_APP_VERSION
#  define JANA4ML4FPGA_APP_VERSION Error
#endif

#define JANA4ML4FPGA_APP_VERSION_STR STR(THIS_APP_VERSION)

namespace jana {

    void PrintUsageOptions() {
        std::cout << "Options:" << std::endl;
        std::cout << "   -h   --help                  Display this message" << std::endl;
        std::cout << "   -v   --version               Display version information" << std::endl;
        std::cout << "   -j   --janaversion           Display JANA version information" << std::endl;
        std::cout << "   -c   --configs               Display configuration parameters" << std::endl;
        std::cout << "   -l   --loadconfigs <file>    Load configuration parameters from file" << std::endl;
        std::cout << "   -d   --dumpconfigs <file>    Dump configuration parameters to file" << std::endl;
        std::cout << "   -L   --list-factories        List all the factories without running" << std::endl;
        std::cout << "   -Pkey=value                  Specify a configuration parameter" << std::endl;
        std::cout << "   -Pplugin:param=value         Specify a parameter value for a plugin" << std::endl;
        std::cout << std::endl;

        std::cout << "   --list-default-plugins       List all the default plugins" << std::endl;
        std::cout << "   --list-available-plugins     List plugins at $JANA4ML4FPGA_PLUGIN_PATH"
                  << std::endl;
        std::cout << std::endl << std::endl;
    }

    void PrintUsageExample() {

        std::cout << "Example:" << std::endl;
        std::cout << "    jana4ml4fpga -Pplugins=plugin1,plugin2,plugin3 -Pnthreads=8 evio_filename.evio" << std::endl;
//        std::cout << "    jana4ml4fpga -Ppodio:print_type_table=1 infile.root" << std::endl << std::endl;
        std::cout << std::endl << std::endl;
    }

    void PrintUsage() {
        /// Prints jana.cc command-line options to stdout, for use by the CLI.
        /// This does not include JANA parameters, which come from
        /// JParameterManager::PrintParameters() instead.

        std::cout << std::endl;
        std::cout << "Usage:" << std::endl;
        std::cout << "    jana4ml4fpga [options] evio_filename1 evio_filename2 ..." << std::endl;
        std::cout << std::endl;

        std::cout << "Description:" << std::endl;
        std::cout << "    Command-line interface for running JANA plugins. This can be used to" << std::endl;
        std::cout << "    read in events and process them. Command-line flags control configuration" << std::endl;
        std::cout << "    while additional arguments denote input files, which are to be loaded and" << std::endl;
        std::cout << "    processed by the appropriate EventSource plugin." << std::endl;
        std::cout << std::endl;

        PrintUsageOptions();
        PrintUsageExample();
    }

    void PrintVersion() {
        std::cout << "JANA4ML4FPGA version: " << JANA4ML4FPGA_APP_VERSION_STR << std::endl;
    }

    void PrintJANAVersion() {
        std::cout << "JANA version: " << JVersion::GetVersion() << std::endl;
    }

    void PrintDefaultPlugins(std::vector<std::string> const& default_plugins) {
        std::cout << "\n List default plugins:\n\n";
        printPluginNames(default_plugins);
        std::cout << std::endl << std::endl;
    }

    void GetPluginNamesInDir(std::set <std::string> &plugin_names, std::string dir_str) {
        // Edge case handler: taking care of invalid and empty dirs
        if (std::filesystem::is_directory(dir_str) == false)
            return;
        if (std::filesystem::is_empty(dir_str))
            return;

        std::string full_path, filename;
        for (const auto &entry: std::filesystem::directory_iterator(dir_str)) {
            full_path = std::string(entry.path());   // Example: "/usr/local/plugins/Tutorial.so"
            filename = full_path.substr(full_path.find_last_of("/") + 1);  // Example: "Tutorial.so"
            if (filename.substr(filename.size() - 3) == ".so") {
                std::string s = filename.substr(0, filename.size() - 3);
    //                std::cout << filename << "==> "  << s << std::endl;
                plugin_names.insert(s);
            }
        }
    }

    /// Get the plugin names by searching for files named as *.so under @var env_var.
    /// @note It does not guarantee any effectiveness of the plugins.
    void GetPluginNamesFromEnvPath(std::set <std::string> &plugin_names, const char *env_var) {
        std::string dir_path;

        const char *env_p = getenv(env_var);
        if (env_p) {
            std::string paths = std::string(env_p);

            std::stringstream envvar_ss(paths);
            while (getline(envvar_ss, dir_path, ':')) {
                GetPluginNamesInDir(plugin_names, dir_path);
            }
        }
    }

    bool HasPrintOnlyCliOptions(UserOptions &options, std::vector <std::string> const &default_plugins) {
        if (options.flags[jana::ShowUsage]) {
            jana::PrintUsage();
            return true;
        }
        if (options.flags[jana::ShowThisVersion]) {
            jana::PrintVersion();
            return true;
        }
        if (options.flags[jana::ShowJANAVersion]) {
            jana::PrintJANAVersion();
            return true;
        }
        if (options.flags[jana::ShowDefaultPlugins]) {
            jana::PrintDefaultPlugins(default_plugins);
            return true;
        }
        return false;
    }

    void AddTimeoutValueToOptionParams(JParameterManager *para_mgr, int val=0) { // FIXME: what the default value?
        // If the user hasn't specified a timeout (on cmd line or in config file), set the timeout to default
        if (para_mgr->FindParameter("jana:timeout") == nullptr) {
            para_mgr->SetParameter("jana:timeout", val);
            para_mgr->SetParameter("jana:warmup_timeout", val); // in seconds
        }
    }

    void AddPrintControlOfOptionListFactoryToOptionParams(JParameterManager *para_mgr, UserOptions &options) {
        // Shut down the [INFO] msg of adding plugins, printing cpu info
        if (options.flags[ListFactories]) {
            para_mgr->SetParameter(
                    "log:off",
                    "JPluginLoader,JArrowProcessingController,JArrow"
            );
        }
    }

    void AddCDAQTopology(JApplication* app) {
        auto topology = app->GetService<JTopologyBuilder>()->create_empty();

        auto source = new EVIOBlockedEventSource;
        auto processor = new EVIOBlockProcessor;

        auto block_queue = new JMailbox<EVIOBlockedEvent *>;
        auto event_queue = new JMailbox <std::shared_ptr<JEvent>>;

        block_queue->set_threshold(1); // For debugging, have it call the disentangler right away
        _DBG_<<"THRESHOLD IS: " << block_queue->get_threshold() << std::endl;

        auto block_source_arrow = new JBlockSourceArrow<EVIOBlockedEvent>("block_source", source, block_queue);
        auto block_disentangler_arrow = new JBlockDisentanglerArrow<EVIOBlockedEvent>(
                "block_disentangler", source, block_queue, event_queue, topology->event_pool);
        auto processor_arrow = new JEventProcessorArrow("processors", event_queue, nullptr, topology->event_pool);

        processor_arrow->add_processor(processor);

        topology->arrows.push_back(block_source_arrow);
        topology->arrows.push_back(block_disentangler_arrow);
        topology->arrows.push_back(processor_arrow);

        topology->sources.push_back(block_source_arrow);
        topology->sinks.push_back(processor_arrow);

        block_source_arrow->attach(block_disentangler_arrow);
        block_disentangler_arrow->attach(processor_arrow);

        app->SetParameterValue("log:trace", "JWorker");
    }

    void AddBlockedEventSourceFromCli(UserOptions &options, std::vector<std::string> &evio_file_sources) {
        for (auto src:options.evio_filenames) {
            evio_file_sources.push_back(src);
        }
    }

    void AddEVIOFileSourceTypeToOptionParams(
            JParameterManager *para_mgr, std::string event_source_type="CDAQEVIOFileSource") {
        if (para_mgr->FindParameter("jana:timeout") == nullptr) {
            para_mgr->SetParameter("event_source_type", event_source_type);
        }
    }

    void AddConfigFromFileToOptionParams(JParameterManager *para_mgr, UserOptions &options) {
        if (options.flags[LoadConfigs]) {
            // If the user specified an external config file, we should definitely use that
            try {
                para_mgr->ReadConfigFile(options.load_config_file);
            }
            catch (JException &e) {
                std::cout << "Problem loading config file '" << options.load_config_file << "'. Exiting." << std::endl
                          << std::endl;
                exit(-1);
            }
            std::cout << "Loaded config file '" << options.load_config_file << "'." << std::endl << std::endl;
        }
    }

    JApplication *CreateJApplication(UserOptions &options) {

        auto para_mgr = new JParameterManager(); // JApplication owns params_copy, does not own eventSources

        // Add the cli options based on the user inputs
        for (auto pair: options.params) {
            para_mgr->SetParameter(pair.first, pair.second);
        }

        AddPrintControlOfOptionListFactoryToOptionParams(para_mgr, options);
        AddTimeoutValueToOptionParams(para_mgr);
        AddConfigFromFileToOptionParams(para_mgr, options);

        auto app = new JApplication(para_mgr);

        return app;
    }

    void AddDefaultPluginsToJApplication(JApplication *app, std::vector<std::string> const &default_plugins) {
        for (std::string s: default_plugins)
            app->AddPlugin(s);
    }

    void PrintFactories(JApplication *app) {
        std::cout << std::endl << "List all the factories:" << std::endl << std::endl;
        printFactoryTable(app->GetComponentSummary());
        std::cout << std::endl;
    }

    void PrintConfigParameters(JApplication *app) {
        /// Print a table of the currently defined configuration parameters.
        /// n.b. this mostly duplicates a call to app->GetJParameterManager()->PrintParameters()
        /// but avoids the issue it has of setting the values column to same
        /// width for all parameters. (That leads to lots of whitespace being
        /// printed due to the very long podio:output_include_collections param.

        // Determine column widths
        auto params = app->GetJParameterManager()->GetAllParameters();
        size_t max_key_length = 0;
        size_t max_val_length = 0;
        size_t max_max_val_length = 32; // maximum width allowed for column.
        for (auto &[key, p]: params) {
            if (key.length() > max_key_length) max_key_length = key.length();
            if (p->GetValue().length() > max_val_length) {
                if (p->GetValue().length() <= max_max_val_length) max_val_length = p->GetValue().length();
            }
        }

        std::cout << "\nConfiguration Parameters:" << std::endl;
        std::cout << "Name" + std::string(max_key_length - 4, ' ') << " : ";
        std::cout << "Value" + std::string(max_val_length - 5, ' ') << " : ";
        std::cout << "Description" << std::endl;
        std::cout << std::string(max_key_length + max_val_length + 20, '-') << std::endl;
        for (auto &[key, p]: params) {
            std::stringstream ss;
            int key_length_diff = max_key_length - key.length();
            if (key_length_diff > 0) ss << std::string(key_length_diff, ' ');
            ss << key;
            ss << " | ";

            int val_length_diff = max_val_length - p->GetValue().length();
            if (val_length_diff > 0) ss << std::string(val_length_diff, ' ');
            ss << p->GetValue();
            ss << " | ";
            ss << p->GetDescription();

            std::cout << ss.str() << std::endl;
        }
        std::cout << std::string(max_key_length + max_val_length + 20, '-') << std::endl;
    }

    int Execute(JApplication *app, UserOptions &options) {

        std::cout << std::endl;

        if (options.flags[ShowConfigs]) {
            // Load all plugins, collect all parameters, exit without running anything
            app->Initialize();
    //            app->GetJParameterManager()->PrintParameters(true);
            PrintConfigParameters(app);
        } else if (options.flags[DumpConfigs]) {
            // Load all plugins, dump parameters to file, exit without running anything
            app->Initialize();
            std::cout << std::endl << "Writing configuration options to file: " << options.dump_config_file
                      << std::endl;
            app->GetJParameterManager()->WriteConfigFile(options.dump_config_file);
        } else if (options.flags[ListFactories]) {
            app->Initialize();
            PrintFactories(app);
        } else {
            // Run JANA in normal mode
            try {
                JSignalHandler::register_handlers(app);

                AddCDAQTopology(app);   /// major change

                app->Run(true);
            }
            catch (JException &e) {
                std::cout << "----------------------------------------------------------" << std::endl;
                std::cout << e << std::endl;
            }
            catch (std::runtime_error &e) {
                std::cout << "----------------------------------------------------------" << std::endl;
                std::cout << "Exception: " << e.what() << std::endl;
            }
        }
        return (int) app->GetExitCode();
    }

    UserOptions GetCliOptions(int nargs, char *argv[], bool expect_extra) {

        UserOptions options;

        std::map <std::string, Flag> tokenizer;
        tokenizer["-h"] = ShowUsage;
        tokenizer["--help"] = ShowUsage;
        tokenizer["-v"] = ShowThisVersion;
        tokenizer["--version"] = ShowThisVersion;
        tokenizer["-j"] = ShowJANAVersion;
        tokenizer["--janaversion"] = ShowJANAVersion;
        tokenizer["-c"] = ShowConfigs;
        tokenizer["--configs"] = ShowConfigs;
        tokenizer["-l"] = LoadConfigs;
        tokenizer["--loadconfigs"] = LoadConfigs;
        tokenizer["-d"] = DumpConfigs;
        tokenizer["--dumpconfigs"] = DumpConfigs;
        tokenizer["-L"] = ListFactories;
        tokenizer["--list-factories"] = ListFactories;
        tokenizer["--list-default-plugins"] = ShowDefaultPlugins;

        // `jana4ml4fpga` has the same effect with `jana4ml4fpga -h`
        if (nargs == 1) {
            options.flags[ShowUsage] = true;
        }

        for (int i = 1; i < nargs; i++) {
            std::string arg = argv[i];
            // std::cout << "Found arg " << arg << std::endl;

            if (argv[i][0] != '-') {  // copy filenames to option.evio_filenames
                options.evio_filenames.push_back(arg);
                continue;
            }

            switch (tokenizer[arg]) {
                case ShowUsage:
                    options.flags[ShowUsage] = true;
                    break;

                case ShowThisVersion:
                    options.flags[ShowThisVersion] = true;
                    break;

                case ShowJANAVersion:
                    options.flags[ShowJANAVersion] = true;
                    break;

                case ShowConfigs:
                    options.flags[ShowConfigs] = true;
                    break;

                case LoadConfigs:
                    options.flags[LoadConfigs] = true;
                    if (i + 1 < nargs && argv[i + 1][0] != '-') {
                        options.load_config_file = argv[i + 1];
                        i += 1;
                    } else {
                        options.load_config_file = "jana.config";
                    }
                    break;

                case DumpConfigs:
                    options.flags[DumpConfigs] = true;
                    if (i + 1 < nargs && argv[i + 1][0] != '-') {
                        options.dump_config_file = argv[i + 1];
                        i += 1;
                    } else {
                        options.dump_config_file = "jana.config";
                    }
                    break;

                case ListFactories:
                    options.flags[ListFactories] = true;
                    break;

                case ShowDefaultPlugins:
                    options.flags[ShowDefaultPlugins] = true;
                    break;

                case Unknown:
                    if (argv[i][0] == '-' && argv[i][1] == 'P') {

                        size_t pos = arg.find("=");
                        if ((pos != std::string::npos) && (pos > 2)) {
                            std::string key = arg.substr(2, pos - 2);
                            std::string val = arg.substr(pos + 1);
                            if (options.params.find(key) != options.params.end()) {
                                std::cout << "Duplicate parameter '" << arg << "' ignored" << std::endl;
                            } else {
                                options.params.insert({key, val});
                            }
                        } else {
                            std::cout << "Invalid JANA parameter '" << arg
                                      << "': Expected format -Pkey=value" << std::endl;
                            options.flags[ShowConfigs] = true;
                        }
                    } else {
                        if (!expect_extra) {
                            std::cout << "Invalid command line flag '" << arg << "'" << std::endl;
                            options.flags[ShowUsage] = true;
                        }
                    }
            }
        }
        return options;
    }

}