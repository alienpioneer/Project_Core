/**
 * @file ServiceManager.hpp
 * @author Alexandru ALEXANDRESCU
 * 
 */

#pragma once
#include <cassert>
#include <thread>
#include <stdexcept>

#include "Config.hpp"
#include "AVector.hpp"
#include "ServiceBaseQueue.hpp"


/**
 * @page ServiceManagerPage ServiceManager
 * 
 * @brief Orchestrates services and manages their threads and lifetime.
 *
 * @section overview Overview
 * `ServiceManager` owns all services (`shared_ptr`) and their threads.
 * @warning Must outlive `EventManager` and all services.
 *
 * @section construction Construction
 * - Requires `std::shared_ptr<EventManager>` at construction.
 * - Owns services created via `registerService<T>()`.
 *
 * @section registration Internal Registration Order
 * 1. `std::make_shared<T>()`
 * 2. Store service in internal container.
 * 3. Call `service->init()`
 * 4. Call `service->start()`
 * 5. Launch service thread.
 *
 * @section lifecycle Lifecycle
 * - stopAll():
 *  - Signals all services to stop.
 *  - Joins all service threads.
 *  - Does NOT stop EventManager.
 *
 * - shutdown():
 *  - Calls stopAll().
 *  - Stops EventManager.
 *  - Safe to call multiple times.
 *
 * - Destructor:
 *  - Calls shutdown() as a fallback.
 *
 * @section constraints Constraints
 * - Services must always be created via std::make_shared
 * - Must not perform subscriptions on behalf of services.
 * - Must not contain service-specific logic.
 * - Only exposes `get<T>()` for testing/debugging.
 * - No detached threads allowed.
 * - Services must communicate via events only; no direct calls.
 * - Never stack-allocate or call `new` without shared ownership.
 *
 * @warning Do not subscribe in: Service constructor, ServiceManager, run() or start() !!
 *
 * @section startup Startup Sequence
 * ```
 * EventManager
 * ServiceManager
 * Services
 * UI
 * ```
 * 
 * @section shutdown Shutdown Sequence
 * ```
 * ServiceManager.shutdown()
 * ├─ stopAll()
 * │   ├─ signal services
 * │   └─ join service threads
 * ├─ EventManager.stop()
 * └─ destroy application objects
 * ```
 * @warning ServiceManager must outlive EventManager and all Services !
 * 
 * @code
 * int main()
 * {
 *     auto eventManager = std::make_shared<Core::EventManager>();
 *     Core::ServiceManager serviceManager(eventManager);
 *     serviceManager.registerService<SensorService>(...);
 *     serviceManager.registerService<LoggerService>(...);
 *     runUiLoop();   // blocks until user exits
 *     // shutdown happens below
 * }
 *  // For tests
 *  auto& w = sm.get<WifiService>(ServiceId::Wifi);
 * @endcode
 */

namespace Core
{

class ServiceManager
{
public:

    static_assert(MAX_SERVICES <= std::numeric_limits<uint8_t>::max());

    /**
     * @brief Construct a ServiceManager.
     * @param manager Shared pointer to EventManager.
     */
    explicit ServiceManager(std::shared_ptr<EventManager> manager)
        : manager_(std::move(manager))
    {}

    /**
     * @brief Destroy the ServiceManager and perform graceful shutdown.
     *
     * Calls shutdown() if it has not already been executed.
     */
    ~ServiceManager();

    ServiceManager(const ServiceManager&) = delete;
    ServiceManager& operator=(const ServiceManager&) = delete;

    /**
     * @brief Register a service.
     * @tparam T Concrete service type derived from ServiceBaseQueue.
     * @tparam Args Constructor arguments for the service.
     * @param args Arguments forwarded to service constructor.
     * @warning ServiceId values must remain < MAX_SERVICES.
     * @warning registerService() must not be called after shutdown().
     * Calls `init()`, `start()`, and launches the service thread.
     */
    template<typename T, typename... Args>
    void registerService(Args&&... args)
    {
        static_assert(std::is_base_of_v<ServiceBaseQueue, T>);

        // Call the service object constructor, which MUST accept a service manager shared_ptr as argument 
        auto service = std::make_shared<T>(std::forward<Args>(args)...,manager_);

        const auto idx = static_cast<std::size_t>(service->id());

        assert(idx < MAX_SERVICES);
        assert(!services_[idx]);

        if (idx >= MAX_SERVICES)
        {
            throw std::runtime_error("CORE : ServiceId exceeds MAX_SERVICES !");
        }

        if (stopped_)
        {
            throw std::runtime_error("CORE : Cannot register services after shutdown");
        }

        services_[idx] = service;

        service->init();
        service->start();

        threadPool_[idx] = std::thread(&T::run, service);
    }

    /**
     * @brief Retrieve a service instance by type.
     * @tparam T Concrete service type.
     * @return shared_ptr to the service.
     * @warning Not meant to be used in production, for tests only !
     */
    template<typename T>
    std::shared_ptr<T> get() const
    {
        static_assert(std::is_base_of_v<ServiceBaseQueue, T>);

        // Each concrete service must declare a static serviceId, for ex: static constexpr ServiceId staticId = ServiceId::Wifi;
        const auto idx = static_cast<std::size_t>(T::staticId);

        return std::static_pointer_cast<T>(services_[idx]);
    }

    /**
     * @brief Stop all services and join threads.
     */
    void stopAll();

    /**
     * @brief Gracefully shutdown the service infrastructure.
     *
     * Performs an ordered shutdown of all managed components:
     * 1. Signals all registered services to stop.
     * 2. Waits for all service threads to terminate.
     * 3. Stops the EventManager dispatcher.
     *
     * Safe to call multiple times; subsequent calls are ignored.
     *
     * @note This is the preferred shutdown mechanism and should normally
     * be called explicitly from application code before destruction.
     *
     * @note Destructor automatically calls shutdown() as a fallback if the
     * application did not perform explicit shutdown.
     *
     * @warning Services must stop before EventManager is stopped.
     * Services may still post events during shutdown.
     *
     * @warning shutdown() blocks until all service threads exit.
     * Services that override run() must periodically check running()
     * and guarantee eventual termination.
     *
     * @warning Must not be called concurrently from multiple threads.
     *
     * Example:
     * @code
     * serviceManager.shutdown();
     * dataManager.shutdown();
     * @endcode
     */
    void shutdown();

private:
    std::shared_ptr<EventManager> manager_;
    std::array<std::shared_ptr<ServiceBaseQueue>, MAX_SERVICES> services_{};
    std::array<std::thread, MAX_SERVICES> threadPool_;
    bool stopped_{false};
};

}// namespace Core
