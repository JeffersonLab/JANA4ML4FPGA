// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <mutex>

#include <JANA/JApplication.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <JANA/Services/JServiceLocator.h>
#include <services/log/Log_service.h>
#include <services/root_output/RootFile_service.h>
#include <JANA/JApplication.h>

/**
 * This Service centralizes creation of Data quality monitor
 */
class GemMappingService : public JService {
public:
    explicit GemMappingService(JApplication *app) : m_app(app) {}

    ~GemMappingService() override = default;

    void acquire_services(JServiceLocator *locater) override {
        auto log_service = m_app->GetService<Log_service>();
        auto root_file_service = m_app->GetService<RootFile_service>();
        m_log = log_service->logger("dqm-service");
        m_root_file = root_file_service->GetHistFile();
        m_top_dir = m_root_file->mkdir("dqm", "Data Quality Monitoring ", /*returnExistingDirectory*/ true);
        m_events_dir = m_top_dir->mkdir("events", "Plots per event", /*returnExistingDirectory*/ true);
        m_integral_dir = m_top_dir->mkdir("integral", "Plots over events range", /*returnExistingDirectory*/ true);

        // Get TDirectory for histograms root file
        m_glb_lock = m_app->GetService<JGlobalRootLock>();



};
