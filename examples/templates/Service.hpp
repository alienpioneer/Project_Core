/**
 * @file Service.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/AVector.hpp"
#include "core/Event.hpp"
#include "core/ServiceBaseQueue.hpp"

/**
 * @page ServiceTemplatePage Service Template
 * @brief Example service implementation using ServiceBaseQueue.
 *
 * ## Overview
 * This class provides a template for implementing a service.
 * It demonstrates:
 * - Subscription in `init()`
 * - Event handling in `dispatch()`
 * - Main loop in `run()`
 *
 * ## Construction
 * - Must receive `EventManager` shared_ptr
 * - Must pass `staticId` to base class
 *
 * ## Lifecycle
 * - `init()` → subscribe to events
 * - `run()` → main loop
 * - `dispatch()` → handle events
 *
 * ## Example
 * @code{.cpp}
 * void Service::dispatch(const Core::Event& event)
 * {
 *     switch(event.type())
 *     {
 *         case Core::EventType::EventX:
 *             // handle event
 *             break;
 *         default:
 *             break;
 *     }
 * }
 * @endcode
 *
 * ## Constraints
 * - Do not subscribe in constructor
 * - Do not block in dispatch()
 * - Do not access other services directly
 */

namespace App
{

class Service final: public Core::ServiceBaseQueue
{
public:
    // Give a valid service Id in Core::ServiceId
    static constexpr Core::ServiceId staticId = Core::ServiceId::Service;

    explicit Service(std::shared_ptr<Core::EventManager> manager, uint8_t maxEventsPerCycle=4);

    ~Service();

    /**
     * @brief Optional Override the run loop
     * @note This will execute in the service thread
     */
    void run() override;

    /**
     * @brief Process the corresponding action depending on the event
     * 
     * @param event 
     */
    void dispatch(const Core::Event& event) override;

    /**
     * @brief Will be called by Service Manager before start
     * @note This is the right place to subscribe to events
     */
    void init() noexcept override;

    /**
     * @brief Metmod that eecutes once in the service thread.
     * Add here methods that need to initilize in the service thread
     * 
     */
    void setup() override;
    
    /**
     * @brief This will be caller periodicaly in the service loop.
     * Add here methods that needs to be executed periodically
     * 
     */
    void update() override;
};

}