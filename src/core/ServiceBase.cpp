/**
 * @file ServiceBase.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceBase.hpp"
#ifndef NDEBUG 
#include <iostream>
#endif

namespace Core
{

// Do NOT call shared_from_this() in this constructor !
ServiceBase::ServiceBase(ServiceId id, std::shared_ptr<EventManager> manager)
    :   id_(id),
        manager_(std::move(manager))
{}

void ServiceBase::subscribe(EventType type) 
{
    manager_->subscribe(type, shared_from_this());
}

void ServiceBase::unsubscribe(EventType type) 
{
    manager_->unsubscribe(type, this);
}

void ServiceBase::sendEvent(EventType type)
{
    manager_->postEvent(Event{type, id_});
}

void ServiceBase::onEvent(const Event& event)
{

#ifndef NDEBUG 
    std::cout << "[Service " << static_cast<int>(id_) << "] received event from "
                << static_cast<int>(event.publisherId()) << std::endl;
#else
    (void)event;
#endif
}

ServiceId ServiceBase::id() const noexcept
{ 
    return id_;
}

}// namespace Core