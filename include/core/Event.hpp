/**
 * @file Event.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include "EventTypes.hpp"
#include "ServiceTypes.hpp"
#include <variant>
#include <string>
#include <string_view>

/**
 * @page EventPage Event
 *
 * @brief Generic message container used for inter-service communication.
 *
 * The Event object represents the fundamental message exchanged between
 * services in the application’s event-driven architecture. Events are
 * published by a service and dispatched through the EventManager to one
 * or more subscribers.
 *
 * An event contains:
 *
 * - a type describing the semantic meaning of the message
 * - the identifier of the publishing service
 * - an optional payload carrying additional data
 *
 * The payload is implemented as a std::variant in order to support
 * multiple lightweight data forms without heap allocations.
 *
 * @section event_structure Structure
 *
 * An Event is composed of a small fixed header and an optional payload.
 *
 * Header fields:
 *
 * - EventType type
 * - ServiceId publisherId
 *
 * Payload:
 *
 * - std::monostate : no payload
 * - int            : simple value (typically a request or correlation id)
 * - Payload        : structured message data
 *
 * The payload variant allows representing small messages efficiently
 * while avoiding dynamic memory allocation.
 *
 * @section event_payload Payload Structure
 *
 * The structured payload is defined as:
 *
 * @code
 * struct Payload
 * {
 *     int dataA;
 *     int dataB;
 *     int dataC;
 *     int dataD;
 *     int requestId;
 * };
 * @endcode
 *
 * This structure is intended for lightweight parameter transport between
 * services when more than a single integer value is required.
 *
 * @section event_usage Usage
 *
 * Creating an event:
 *
 * @code
 * Event e(EventType::REQUEST, ServiceId::UI);
 * @endcode
 *
 * Setting a simple payload:
 *
 * @code
 * e.setPayload(42);
 * @endcode
 *
 * Setting a structured payload:
 *
 * @code
 * Event::Payload p;
 * p.dataA = 1;
 * p.dataB = 2;
 * p.requestId = 10;
 *
 * e.setPayload(p);
 * @endcode
 *
 * Never use raw std::get for the payload variant. This causes std::bad_variant_access: std::get<int>(payload_);
 * 
 * Reading the payload safely:
 *
 * @code
 * if (auto id = std::get_if<int>(&event.payload()))
 * {
 *     // use *id
 * }
 * else if (auto p = std::get_if<Event::Payload>(&event.payload()))
 * {
 *     // use structured payload
 * }
 * @endcode
 * 
 * You can also use the std::visitor to access std::variant data.
 *
 * @section event_design Design Notes
 *
 * - Events are lightweight value objects intended for frequent creation
 *   and dispatch.
 * - The payload variant avoids dynamic allocation and keeps the object
 *   cache-friendly.
 * - std::monostate represents the absence of payload.
 * - Safe access to payload alternatives should use std::get_if or
 *   std::visit to avoid std::bad_variant_access.
 *
 * This design allows efficient message passing between services while
 * keeping the Event object generic and independent from service-specific
 * data types.
 */

namespace Core
{

class Event;

/**
 * @brief The interface class for the subscribers
 * 
 */
class ISubscriber 
{
    public:
        virtual ~ISubscriber() = default;
        virtual void onEvent(const Event& event) = 0;
};

class Event
{

public:

    struct Payload
    {
        int dataA = 0;
        int dataB = 0;
        int dataC = 0;
        int dataD = 0;
        int requestId = 0;
    };

    Event() = default;
    virtual ~Event() = default;

    Event(EventType t, ServiceId id);

    explicit Event(EventType t);

    /**
     * @brief Get event type
     * 
     * @return const EventType& 
     */
    const EventType& type() const;

    /**
     * @brief Get the event publisher id
     * 
     * @return const ServiceId& 
     */
    const ServiceId& publisherId() const;

    /**
     * @brief Get the event payload
     * 
     * @return const std::variant<std::monostate, int, std::string, Payload>& 
     */
    const std::variant<std::monostate, int, std::string, Payload>& payload() const noexcept;

    /**
     * @brief Get the event payload
     * 
     * @return std::variant<std::monostate, int, std::string_view, Payload>& 
     */
    std::variant<std::monostate, int, std::string, Payload>& payload() noexcept;

    /**
     * @brief Set the Event int payload
     * 
     * @param v 
     */
    void setPayload(int v) noexcept;                

    /**
     * @brief Set the Event string_view payload
     * 
     * @param v 
     */
    void setPayload(std::string_view v) noexcept;

    /**
     * @brief Set the Event Payload object
     * 
     * @param v 
     */
    void setPayload(const Payload& v) noexcept;

    /**
     * @brief Set the Event rvalue Payload object
     * 
     * @param v 
     */
    void setPayload(Payload&& v) noexcept;

    /**
     * @brief Reset the event id - useful for testing
     * 
     * @param id 
     */
    void resetId(const ServiceId id) noexcept;

private:
    EventType type_{EventType::DEFAULT};
    ServiceId publisherId_{ServiceId::BASE};
    std::variant<std::monostate, int, std::string, Payload> payload_;
};


} // namespace Core

