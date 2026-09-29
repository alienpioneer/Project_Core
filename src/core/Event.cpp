/**
 * @file Event.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/Event.hpp"

namespace Core
{

Event::Event(EventType t, ServiceId id)
    :   type_{t},
        publisherId_{id}
{}

Event::Event(EventType t)
    :   type_{t}
{}

const EventType& Event::type() const
{
    return type_; 
}

const ServiceId& Event::publisherId() const
{
    return publisherId_; 
}

const std::variant<std::monostate, int, std::string, Event::Payload>& Event::payload() const noexcept
{
    return payload_;
}

std::variant<std::monostate, int, std::string, Event::Payload>& Event::payload() noexcept
{
    return payload_;
}

void Event::setPayload(int v) noexcept                
{ 
    payload_ = v; 
}

void Event::setPayload(std::string_view v) noexcept
{
    payload_ = std::string(v); 
}

void Event::setPayload(const Event::Payload& v) noexcept
{ 
    payload_ = v; 
}

void Event::setPayload(Event::Payload&& v) noexcept
{ 
    payload_ = std::move(v); 
}

void Event::resetId(const ServiceId id) noexcept
{
    publisherId_ = id;
}

} // namespace Core
