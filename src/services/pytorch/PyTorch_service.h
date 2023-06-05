#pragma once


#include <iostream>
#include <vector>
#include <string>

#include <JANA/JApplication.h>
#include <JANA/Services/JServiceLocator.h>

#include <spdlog/spdlog.h>

/**
 * The Service centralizes use of spdlog
 */
class PyTorch_service : public JService
{
public:
    explicit PyTorch_service(JApplication *app );

    std::shared_ptr<spdlog::logger> logger(const std::string &name);

    /** Gets the default level for all loggers
     * The Log level is set from user parameters or is 'info'**/
    spdlog::level::level_enum getDefaultLevel();

    /** Gets std::string version of the default Log level **/
    std::string getDefaultLevelStr();


private:

    PyTorch_service()=default;

    std::recursive_mutex m_lock;
    JApplication* m_application;
    std::string m_log_level_str;
};

