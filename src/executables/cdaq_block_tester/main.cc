
// Refer to JANA

#include <JANA/JApplication.h>
#include <JANA/Engine/JTopologyBuilder.h>
#include <JANA/Engine/JBlockSourceArrow.h>
#include <JANA/Engine/JBlockDisentanglerArrow.h>
#include <JANA/Engine/JEventProcessorArrow.h>

#include "EVIOBlock.h"
#include "EVIOBlockedSource.h"
#include "EVIOBlockProcessor.h"


int main() {

    JApplication app;
    auto topology = app.GetService<JTopologyBuilder>()->create_empty();

    auto source = new EVIOBlockedSource;
    auto processor = new EVIOBlockProcessor();

    auto block_queue = new JMailbox<EVIOBlock *>;
    auto event_queue = new JMailbox <std::shared_ptr<JEvent>>;

    auto block_source_arrow = new JBlockSourceArrow<EVIOBlock>("block_source", source, block_queue);
    auto block_disentangler_arrow = new JBlockDisentanglerArrow<MyBlock>("block_disentangler", source, block_queue,
                                                                         event_queue, topology->event_pool);
    auto processor_arrow = new JEventProcessorArrow("processors", event_queue, nullptr, topology->event_pool);

    processor_arrow->add_processor(processor);

    topology->arrows.push_back(block_source_arrow);
    topology->arrows.push_back(block_disentangler_arrow);
    topology->arrows.push_back(processor_arrow);

    topology->sources.push_back(block_source_arrow);
    topology->sinks.push_back(processor_arrow);

    block_source_arrow->attach(block_disentangler_arrow);
    block_disentangler_arrow->attach(processor_arrow);

    app.SetParameterValue("log:trace", "JWorker");
    app.Run(true);
}

