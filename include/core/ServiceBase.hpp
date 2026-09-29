/**
 * @file ServiceBase.hpp
 * @author Alexandru ALEXANDRESCU
 * 
 */

#pragma once   
#include "EventManager.hpp"
#include "ServiceTypes.hpp"


 /**
 * @page ServiceBasePage ServiceBase
 * 
 * @brief Service base for synchronous communication
 *
 * @section sb_overview Overview
 *
 * ServiceBase is the foundation for synchronous components within the event-driven system. It provides the core interface for services to interact with the EventManager without requiring internal threading or mailbox logic.
 *
 * @section sb_lifecycle Lifecycle and Ownership
 *
 * Every ServiceBase instance must be managed by a std::shared_ptr because it inherits from std::enable_shared_from_this. This allows the EventManager to store weak references to the service, preventing circular ownership.
 *
 * @code
 * class ServiceBase : public ISubscriber, public std::enable_shared_from_this<ServiceBase>
 * @endcode
 *
 * @section sb_dispatch Execution Model
 *
 * Because ServiceBase does not possess an internal queue, the onEvent callback is executed synchronously within the EventManager dispatcher thread. Implementation logic must be non-blocking and highly efficient to ensure the entire system remains responsive.
 *
 * @section sb_usage Basic Usage
 *
 * 1. Inherit from ServiceBase.
 * 2. Override onEvent to handle incoming data.
 * 3. Call subscribe only after the shared pointer for the service has been created.
 *
 * @code
 * void onEvent(const Event& event) override;
 * @endcode
 * 
 * A service template is available at @ref TemplatesPage Templates
 */

namespace Core
{

class ServiceBase : public ISubscriber, public std::enable_shared_from_this<ServiceBase> 
{
public:
    explicit ServiceBase(ServiceId id, std::shared_ptr<EventManager> manager);

    virtual ~ServiceBase() noexcept = default;

    ServiceBase(const ServiceBase&) = delete;
    ServiceBase& operator=(const ServiceBase&) = delete;

    // Subscribe to an event type (call AFTER creating the service shared_ptr)
    void subscribe(EventType type);

    // Unsubscribe from an event type
    void unsubscribe(EventType type);

    // Post an event to the EventManager with optional payload
    void sendEvent(EventType type);

    // Default event handler (can be overridden in derived services)
    void onEvent(const Event& event) override;

    ServiceId id() const noexcept;

private:
    ServiceId id_;
    std::shared_ptr<EventManager> manager_;
};

}// namespace Core

