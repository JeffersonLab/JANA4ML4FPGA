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
#include "GemMapping.h"

/**
 * This Service centralizes creation of Data quality monitor
 */
 namespace ml4fpga::gem {
     class GemMappingService : public JService {
     public:
         explicit GemMappingService(JApplication *app) : m_app(app) {}

         ~GemMappingService() override = default;

         void acquire_services(JServiceLocator *locater) override {
             auto log_service = m_app->GetService<Log_service>();
             auto root_file_service = m_app->GetService<RootFile_service>();
             m_log = log_service->logger("gemrecon:MappingService");
             m_mapping = GemMapping::GetInstance();

             // I N I T   M A P P I N G
             std::string mapping_file = "mapping.cfg";
             m_app->SetDefaultParameter("gemrecon:mapping", mapping_file, "Full path to gem config");
             m_log->info("Mapping file file: {}", mapping_file);
             m_mapping->LoadMapping(mapping_file.c_str());
             m_mapping->PrintMapping();

             auto plane_map = m_mapping->GetAPVIDListFromPlaneMap();
             m_log->info("MAPPING EXPLAINED:");
             for (auto pair: plane_map) {
                 auto name = pair.first;
                 auto det_name = m_mapping->GetDetectorFromPlane(name);
                 auto apv_list = pair.second;
                 m_log->info("  Plane: {:<10} from detector {:<10} has {} APVs:", name, det_name, apv_list.size());
                 for (auto apv_id: apv_list) {
                     m_log->info("    {}", apv_id);
                 }
             }
         }

         GemMapping* GetMapping() { return m_mapping; }

     private:
         JApplication *m_app = nullptr;
         std::shared_ptr<spdlog::logger> m_log;
         GemMapping* m_mapping;
     };
 }