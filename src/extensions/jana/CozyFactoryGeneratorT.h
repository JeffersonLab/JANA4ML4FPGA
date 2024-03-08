// Copyright 2023, Jefferson Science Associates, LLC.
// Subject to the terms in the LICENSE file found in the top-level directory.
// Created by Nathan Brei

#pragma once

#include <JANA/JFactorySet.h>
#include <JANA/JFactoryGenerator.h>
#include <vector>


template<class FactoryT>
class CozyFactoryGeneratorT : public JFactoryGenerator {
public:

    explicit CozyFactoryGeneratorT(JApplication* app) {
        m_app = app;
    };

    explicit CozyFactoryGeneratorT(std::string tag, JApplication* app) {
        m_tag = tag;
        m_app = app;
    }


    void GenerateFactories(JFactorySet *factory_set) override {
        FactoryT *factory = new FactoryT;
        factory->SetApplication(m_app);
        factory->SetPluginName(this->GetPluginName());
        factory->SetFactoryName(JTypeInfo::demangle<FactoryT>());

        // Set up all of the wiring prereqs so that Init() can do its thing
        // Specifically, it needs valid input/output tags, a valid logger, and
        // valid default values in its Config object
        factory->PreInit(m_tag);

        // Factory is ready
        factory_set->Add(factory);

    }

private:

    JApplication* m_app;
    std::string m_tag;

};
