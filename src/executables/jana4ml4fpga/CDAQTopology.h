//
// Created by xmei on 3/8/23.
//

#ifndef JANA4ML4FPGA_CDAQTOPOLOGY_H
#define JANA4ML4FPGA_CDAQTOPOLOGY_H

#include <JANA/JApplication.h>
#include <JANA/Engine/JTopologyBuilder.h>
#include <JANA/Engine/JBlockSourceArrow.h>
#include <JANA/Engine/JBlockDisentanglerArrow.h>
#include <JANA/Engine/JEventProcessorArrow.h>

#include <rawdataparser/EVIOBlockedEvent.h>

#include "EVIOBlockProcessor.h"
#include "EVIOBlockedEventFileSource.h"

std::shared_ptr<JArrowTopology> configureTopology(
        std::shared_ptr<JArrowTopology> topology,
        std::vector<std::string> evio_block_event_srcs
        ) {

    auto source = new EVIOBlockedEventFileSource(evio_block_event_srcs);
    auto processor = new EVIOBlockProcessor;

    auto block_queue = new JMailbox<EVIOBlockedEvent *>;
    auto event_queue = new JMailbox<std::shared_ptr<JEvent>>;

    block_queue->set_threshold(1); // For debugging, have it call the disentangler right away
//        _DBG_<<"THRESHOLD IS: " << block_queue->get_threshold() << std::endl;

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

    // If you want to add additional processors loaded from plugins, do this like so:
    for (auto proc : topology->component_manager->get_evt_procs()) {
        processor_arrow->add_processor(proc);
    }

    // If you want to add additional sources loaded from plugins, you'll also need to set up a JEventSourceArrow.
    // Look at JTopologyBuilder::create_default_topology() for an example.
    /// TODO (@xmei): add normal EventSource here

    return topology;
}


void addCDAQTopology(JApplication* app, std::vector<std::string> evio_block_event_srcs) {

    app->GetService<JTopologyBuilder>()->set_configure_fn(
        [=](std::shared_ptr<JArrowTopology> topo){
            return configureTopology(topo, evio_block_event_srcs);
        }
    );
    app->SetParameterValue("log:trace", "JWorker");
}

#endif //JANA4ML4FPGA_CDAQTOPOLOGY_H
