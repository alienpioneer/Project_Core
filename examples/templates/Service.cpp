/**
 * @file Service.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <Service.hpp>

/**
 * @page ServiceTemplateImplementation Service Template Implementation
 * @brief Reference implementation for service behavior.
 *
 * ## Run Loop
 * @code{.cpp}
 * while(running())
 * {
 *     update();
 *     dispatchLoop();
 *     dispatchLoopBlocking();
 *     sleep(...)
 * }
 * @endcode
 *
 * ## Notes
 * - `dispatchLoop()` is non-blocking
 * - `dispatchLoopBlocking()` is optional
 * - `update()` is for periodic work
 * - `setup()` runs once before loop
 */

namespace App
{

/**
 * @brief Construct a new Service:: Service object
 * 
 * @param manager 
 * @param maxEventsPerCycle 
 * @warning Do not subscribe in the constructor !
 * @warning Do not call shared_from_this() in the constructor!
 */
Service::Service(std::shared_ptr<Core::EventManager> manager, uint8_t maxEventsPerCycle)
    :   ServiceBaseQueue(staticId, std::move(manager), maxEventsPerCycle)
{

}

Service::~Service() = default;

/**
 * @brief Will be called by Service Manager before start
 * @note This is the right place to subscribe to events
 */
void Service::init() noexcept
{
    // subscribe(Core::EventType::SpecificEvent);
}

/**
 * @brief Optional Override the run loop
 * @note This will execute in the service thread
 */
void Service::run()
{
    // Add here extra logic to run in the service dedicated thread
    setup();

    while(running())
    {
        update();

        dispatchLoop();

        // Use this in the services where dispatching events first is more important than service work !
        dispatchLoopBlocking();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

/**
 * @brief Main function used to dispatch events
 * 
 * @param event 
 */
void Service::dispatch(const Core::Event& event)
{
    switch(event.type())
    {
        case Core::EventType::EventX:
            // onEventX();
            break;
        default: 
            break;
    }
}

/**
 * @brief Metmod that eecutes once in the service thread.
 * Add here methods that need to initilize in the service thread
 * 
 */
void Service::update()
{

}

/**
 * @brief This will be caller periodicaly in the service loop.
 * Add here methods that needs to be executed periodically
 * 
 */
void Service::setup()
{

}

}// namespace App