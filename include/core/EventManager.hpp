/**
 * @file EventManager.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once   
#include "Event.hpp"
#include <memory>

/**
 * @page EventManagerPage EventManager
 *
 * @brief The EventManager is the central structure responsible for delivering events to subscribers.
 *
 * Responsibilities:
 *
 * - Maintain subscriber lists for each EventType.
 * - Provide thread-safe event posting.
 * - Dispatch events from a dedicated dispatcher thread.
 *
 * Internally the EventManager contains:
 *
 * - a dispatcher thread
 * - a thread-safe event queue
 * - a subscriber registry:
 *
 * @code
 * std::array<
 *     AVector<std::weak_ptr<ISubscriber>, EVENTS_VECTOR_MAX_SUBSCRIBERS>,
 *     static_cast<std::size_t>(EventType::EVENT_COUNT)
 * > subscribers_;
 * @endcode
 *
 * Subscribers are stored as weak pointers to avoid ownership cycles.
 *
 * Dispatching occurs outside the subscriber lock by using a temporary
 * snapshot of valid subscribers.
 *
 * 
 * @section design_goals Design Goals
 *
 * - Strong service isolation
 * - Thread-safe event communication
 * - Deterministic shutdown
 * - No detached threads
 * - RAII-based lifecycle management
 * - Minimal coupling between components
 *
 * @warning The EventManager must remain alive while services may still publish events.
 * 
 * @note EventManager is a process-lifetime object and must outlive all services.
 * 
 * @note You would need shared_from_this() only if you later introduce one of these:
 *  - Dynamic teardown / restart of EventManager
 *  - Multiple EventManager instances with transferable ownership
 *  - Fire-and-forget async tasks capturing this beyond the caller’s scope
 * 
 * @note EventManager lifetime must be:
 *  ->start before services
 *  ->stop after services
 * 
 */

namespace Core
{

class EventManager
{
public:

    EventManager();
    ~EventManager() noexcept;

    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    /**
     * @brief Subscribe to the event type
     * 
     * @param type 
     * @param subscriber 
     */
    void subscribe(EventType type, const std::shared_ptr<ISubscriber>& subscriber);

    /**
     * @brief Unsubcribe from the event type
     * 
     * @param type 
     * @param subscriber 
     */
    void unsubscribe(EventType type, const ISubscriber* subscriber);

    /**
     * @brief Post an event
     * 
     * @param event 
     */
    void postEvent(Event event);

    /**
     * @brief Stop event manager
     * 
     */
    void stop() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace Core
