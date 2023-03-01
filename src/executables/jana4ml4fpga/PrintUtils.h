//
// Created by xmei on 2/27/23.
// Taken from eicrecon
//

#ifndef JANA4ML4FPGA_APPPRINTER_H
#define JANA4ML4FPGA_APPPRINTER_H

#pragma once

#include <iostream>
#include <iomanip>
#include <JANA/Utils/JTablePrinter.h>
#include <JANA/Services/JComponentManager.h>

void printFactoryTable(JComponentSummary const &cs) {
    JTablePrinter factory_table;
    factory_table.AddColumn("Plugin");
    factory_table.AddColumn("Object name");
    factory_table.AddColumn("Tag");
    for (const auto &factory: cs.factories) {
        factory_table | factory.plugin_name | factory.object_name | factory.factory_tag;
    }

    std::ostringstream ss;
    factory_table.Render(ss);
    std::cout << ss.str() << std::endl;
}

void printPluginNames(std::vector<std::string>const& plugin_names) {
    JTablePrinter plugin_table;
    plugin_table.AddColumn("Plugin name");
    for (const auto &p: plugin_names) {
        plugin_table | p;
    }

    std::ostringstream ss;
    plugin_table.Render(ss);
    std::cout << ss.str()<< std::endl;
}

void printHeaderIMG() {
    std::cout << std::endl;
    std::cout << "                             #                     #                                   \n"
                 "      #   ##   #    #   ##   #    #  #    # #      #    #  ###### #####   ####    ##   \n"
                 "      #  #  #  ##   #  #  #  #    #  ##  ## #      #    #  #      #    # #    #  #  #  \n"
                 "      # #    # # #  # #    # #    #  # ## # #      #    #  #####  #    # #      #    # \n"
                 "      # ###### #  # # ###### ####### #    # #      ####### #      #####  #  ### ###### \n"
                 " #    # #    # #   ## #    #      #  #    # #           #  #      #      #    # #    # \n"
                 "  ####  #    # #    # #    #      #  #    # ######      #  #      #       ####  #    # \n\n\n";
}

#endif //JANA4ML4FPGA_APPPRINTER_H
