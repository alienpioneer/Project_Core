/**
 * @file ServiceBaseQueue.hpp
 * @author Alexandru ALEXANDRESCU
 * 
 */

#pragma once   
#include "EventManager.hpp"
#include "ServiceTypes.hpp"


/**
 * @page ServiceBaseQueuePage ServiceBaseQueue
 * 
 * @brief Thread-isolated service base class with internal event queue.
 *
 * @section overview Overview
 * `ServiceBaseQueue` provides a thread-isolated service model with a mailbox (internal queue)
 * and optional event dispatching. Services communicate only via events.
 *
 * @section construction Construction & Initialization
 * - Must be created via `std::make_shared<T>()`.
 * - Must not call `shared_from_this()` in constructor or destructor.
 * - `init()` must be called after construction (by `ServiceManager`) to subscribe to events.
 * - All subscribe() calls must occur inside init();
 *
 * @section threading Threading
 * - `onEvent()` runs in the EventManager dispatcher thread.
 * - `onEvent()` must be non-blocking.
 * - `dispatch()` runs only in the service thread.
 * - `dispatch()` must never execute under the queue mutex (must not assume external synchronization) .
 * - `dispatchLoop()` must not block in the non-blocking model.
 * - `run()` controls pacing (sleep, wait, timers).
 *
 * @section lifecycle Lifecycle
 * - `init()` optional initialization method that must be called after the constructor have been called - use it for subscriptions
 * - `setup()` optional setup method executed once after start, inside the service thread - use it for initialization in the working thread !
 * - `update()` optional method executed periodically inside the worker thread.
 * - `run()` exits when `running_ == false` and queue is empty.
 * - `start()` must be called before entering `run()` loop.
 * - `stop()` signals termination and unblocks waiting threads.
 *
 * @section event_semantics Event Semantics
 * - Routing is based solely on `EventType`.
 * - `ServiceId` is metadata (e.g., sender filtering) only.
 * - Filtering by sender must be handled inside dispatch().
 *
 * @section constraints Constraints
 * - Do not subscribe in constructor or `ServiceManager`.
 * - `dispatchLoop()` MUST be non-blocking; use `dispatchLoopBlocking()` for explicit blocking.
 * - `dispatch()` must not execute under queue mutex.
 * - Services must not call each other directly.
 * - Service main loop must avoid busy spinning.
 * - Event processing is opportunistic, not guaranteed immediate.
 * - Shutdown must rely on running_ variable only (no CV wake dependency)
 * 
 * @section example Example Usage (Derived Service)
 * @code
 * ```
 * 
 * void MyService::run()
 * {
 *     setup();
 * 
 * 
 *     while (running())
 *    {
 *         dispatchLoop(4);
 *         // optional periodic work
 *     }
 * }
 * ...
 * start();
 * ```
 * @endcode
 * 
 * ## Correct pattern in ServiceManager :
 * 
 * @code
 * ```
 * service->init();
 * service->start();
 * threads_[idx] = std::thread([service] {
 *     service->run();
 * });
 *
 * ```
 * @endcode
 * 
 */

namespace Core
{

class ServiceBaseQueue : public ISubscriber, public std::enable_shared_from_this<ServiceBaseQueue> 
{
public:

    /**
     * @brief Construct a service with given id and event manager.
     * @param id Service identifier.
     * @param manager Shared pointer to EventManager.
     * @param maxEventsPerCycle Maximum events processed per dispatch loop cycle.
     *
     * @note Do NOT call `shared_from_this()` in constructor.
     * @warning DO NOT subscribe in the service constructor !
     */
    explicit ServiceBaseQueue(ServiceId id, std::shared_ptr<EventManager> manager, uint8_t maxEventsPerCycle=4);

    /**
     * @brief Destroy the service.
     */
    virtual ~ServiceBaseQueue() noexcept;

    ServiceBaseQueue(const ServiceBaseQueue&) = delete;
    ServiceBaseQueue& operator=(const ServiceBaseQueue&) = delete;

    /**
     * @brief Subscribe to an event type. Must be called after `init()`.
     * @param type Event type to subscribe.
     * @warning Must call AFTER creating the service shared_ptr (after calling the constructor)
     * @warning MUST NOT be called in the constructor !
     */
    void subscribe(EventType type);

    /**
     * @brief Unsubscribe from an event type.
     * @param type Event type to unsubscribe.
     */
    void unsubscribe(EventType type);

     /**
     * @brief Post an event with no payload.
     * @param type Event type to post.
     */
    void sendEvent(EventType type);

    /**
     * @brief Post an event with optional payload.
     * @param e Event object to post.
     */
    void sendEvent(Event e);

    // Default event handler (can be overridden in derived services)
    /**
     * @brief Callback executed by EventManager thread when an event is delivered. 
     * @param event Event received.
     * @note Can be overridden in derived services
     */
    void onEvent(const Event& event) override;

    /**
     * @brief Returns the service identifier.
     */
    ServiceId id() const noexcept;

    /**
     * @brief Returns true if the service is running.
     */
    bool running() const noexcept;

    /**
     * @brief Stop the service.
     */
    void stop() noexcept;

    /**
     * @brief Start the service and the message dispatching
     */
    void start() noexcept;

    /**
     * @brief Optional initialization after construction, e.g., subscribing to events.
     * @note Called by ServiceManager.
     * @warning Must be called after the constructor have been called - use it for subscriptions
     */
    virtual void init() noexcept {}; 

    /**
     * @brief Setup routine executed once after start, inside the service thread.
     * @note To be executed in the run() loop
     * @note Can be overridden.
     */
    virtual void setup(){};

    /**
     * @brief Dispatch a single event. Called in service thread.
     * Must not execute under queue mutex.
     * @param event Event to process.
     * @note Process the corresponding action depending on the event
     */
    virtual void dispatch(const Event& event) = 0;

    /**
     * @brief Optional periodic service update executed in the main loop.
     * @note Can be used on periodic tasks
     */
    virtual void update(){};

    /**
     * @brief Main service loop.
     * @note Can be overridden.
     */
    virtual void run();


protected:

    /**
     * @brief Process events in non-blocking mode (up to maxEventsPerCycle_).
     * @warning TO BE PLACED IN run() loop, once implemented !
     */
    void dispatchLoop();

    /**
     * @brief Process events in blocking mode (up to maxEventsPerCycle_).
     * @warning Same as dispatchLoop but blocking - treats all the messages at once !
     */
    void dispatchLoopBlocking();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    uint8_t maxEventsPerCycle_;
};

}// namespace Core
