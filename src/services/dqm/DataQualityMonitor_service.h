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

#include <TFile.h>

/**
 * This Service centralizes creation of Data quality monitor
 */
class DataQualityMonitor_service : public JService
{
public:
    explicit DataQualityMonitor_service(JApplication *app ):m_app(app){}
    ~DataQualityMonitor_service() override { CloseHistFile(); }

    void acquire_services(JServiceLocator *locater) override {
        auto log_service = m_app->GetService<Log_service>();
        auto root_file_service = m_app->GetService<RootFile_service>();
        m_log = log_service->logger("DQM");
        m_root_file = root_file_service->GetHistFile();
        m_top_dir = m_root_file->mkdir("dqm", "Data Quality Monitoring ", /*returnExistingDirectory*/ true);
        m_events_dir = m_top_dir->mkdir("events", "Plots per event", /*returnExistingDirectory*/ true);

        // Get TDirectory for histograms root file
        m_glb_lock = m_app->GetService<JGlobalRootLock>();
    }

    /// This will return a pointer to the top-level directory for current file
    ///
    /// \return main DQM directory
    TDirectory* GetEventDir(uint64_t entry_num){
        m_glb_lock->acquire_write_lock();
        std::string dir_name = "evt_" + std::to_string(entry_num);
        auto result  = m_events_dir->mkdir(dir_name.c_str(), /*title=*/"", /*returnExistingDirectory=*/true);
        m_glb_lock->release_lock();
        return result;
    }

private:

    DataQualityMonitor_service()=default;

    JApplication *m_app=nullptr;
    std::shared_ptr<spdlog::logger> m_log;
    TDirectory *m_root_file;                        /// pointer to a TDirectory object which represents the ROOT file.
    TDirectory *m_top_dir;                          /// top DQM directory
    TDirectory *m_events_dir;                       /// TDirectory where each event subdir is stored
    std::shared_ptr<JGlobalRootLock> m_glb_lock;    /// Global ROOT lock
};

