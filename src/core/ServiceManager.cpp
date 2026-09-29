/**
 * @file ServiceManager.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceManager.hpp"

namespace Core
{

ServiceManager::~ServiceManager()
{
    shutdown();
}

void ServiceManager::stopAll()
{
    // signal stop
    for (auto& svc : services_)
    {
        if (svc)
        {
            svc->stop();
        }
    }

    // join threads
    for (auto& t : threadPool_)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

void ServiceManager::shutdown()
{
    if (stopped_)
        return;

    stopped_ = true;

    stopAll();

    if (manager_)
    {
        manager_->stop();
    }
}

}// namespace Core