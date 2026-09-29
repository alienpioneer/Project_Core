/**
 * @file test_stress_event_manager.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/EventManager.hpp"
#include "core/ServiceBase.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

namespace TestStressEventManager
{
    // Dummy service that counts events
class StressService : public Core::ServiceBase 
{
public:
    StressService(Core::ServiceId id, std::shared_ptr<Core::EventManager> manager)
        : ServiceBase(id, std::move(manager)),
          receivedCount(0) 
    {}

    void onEvent(const Core::Event& /*event*/) override 
    {
        receivedCount.fetch_add(1, std::memory_order_relaxed);
    }

    std::atomic<int> receivedCount;
};

TEST(EventManagerStressTest, ConcurrentSubscribeUnsubscribe)
{
    auto manager = std::make_shared<Core::EventManager>();

    constexpr int iterations = 1000;
    constexpr int threads = 4;

    std::vector<std::shared_ptr<StressService>> services;
    std::vector<std::thread> workers;

    for (int i = 0; i < 10; ++i)
    {
        services.push_back(std::make_shared<StressService>(Core::ServiceId::BASE, manager));
    }

    for (int t = 0; t < threads; ++t)
    {
        workers.emplace_back([&, t] {
            for (int i = 0; i < iterations; ++i)
            {
                auto& svc = services[i % services.size()];
                svc->subscribe(Core::EventType::TestEvent);
                svc->unsubscribe(Core::EventType::TestEvent);
            }
        });
    }

    for (auto& workerThread : workers)
    {
        workerThread.join();
    }
        
    SUCCEED(); // Expect no crash, no deadlock
}

TEST(EventManagerStressTest, UnsubscribeWhileDispatching)
{
    auto manager = std::make_shared<Core::EventManager>();
    auto service = std::make_shared<StressService>(Core::ServiceId::TestService, manager);
    std::atomic<bool> running{true};

    service->subscribe(Core::EventType::TestEvent);

    std::thread producer([&] {
        while (running)
        {
            manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
        }
    });

    std::thread toggler([&] {
        for (int i = 0; i < 1000; ++i)
        {
            service->unsubscribe(Core::EventType::TestEvent);
            service->subscribe(Core::EventType::TestEvent);
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    toggler.join();

    running = false;
    manager->postEvent(Core::Event{Core::EventType::SHUTDOWN, Core::ServiceId::BASE});

    producer.join();

    SUCCEED(); // survived concurrent unsubscribe + dispatch
}

TEST(EventManagerStressTest, ManyServicesSubscribeUnsubscribeWhileDispatching)
{
    auto manager = std::make_shared<Core::EventManager>();
    std::vector<std::shared_ptr<StressService>> services;
    std::vector<std::thread> workers;
    std::atomic<bool> running{true};

    constexpr int serviceCount = 6;
    constexpr int iterations = 500;

    for (int i = 0; i < serviceCount; ++i)
    {
        services.push_back(std::make_shared<StressService>(Core::ServiceId::TestService, manager));
    }

    std::thread producer([&] {
        while (running)
        {
            manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
        }
    });

    for (auto& svc : services)
    {
        workers.emplace_back([&, svc] {
            for (int i = 0; i < iterations; ++i)
            {
                svc->subscribe(Core::EventType::TestEvent);
                svc->unsubscribe(Core::EventType::TestEvent);
            }
        });
    }

    for (auto& th : workers)
    {
        th.join();
    }
        
    running = false;
    producer.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    SUCCEED(); // survived concurrent subscribe + unsubscribe + dispatch
}

TEST(EventManagerStressTest, RepeatedSubscribeSameService)
{
    auto manager = std::make_shared<Core::EventManager>();
    auto service = std::make_shared<StressService>(Core::ServiceId::TestService, manager);

    for (int i = 0; i < 1000; ++i)
    {
        service->subscribe(Core::EventType::TestEvent);
    }

    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    EXPECT_GE(service->receivedCount.load(), 1);
}

TEST(EventManagerStressTest, MultiThreadedPost)
{
    constexpr int numServices = 5;
    constexpr int numThreads = 4;
    // If you go any higher than eventsPerThread = 10000,
    // increase QUEUE_MAX_SIZE and EVENTS_VECTOR_MAX_SUBSCRIBERS in project_config.cmake
    constexpr int eventsPerThread = 10000;

    constexpr int expected = numThreads * eventsPerThread;

    auto manager = std::make_shared<Core::EventManager>();
    std::vector<std::shared_ptr<StressService>> services;
    std::vector<std::thread> workers;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);

    for (int i = 0; i < numServices; ++i) 
    {
        auto svc = std::make_shared<StressService>(Core::ServiceId::TestService, manager);
        svc->subscribe(Core::EventType::TestEvent);
        services.push_back(svc);
    }

    for (int t = 0; t < numThreads; ++t) 
    {
        workers.emplace_back([manager, t]()
        {
            for (int i = 0; i < eventsPerThread; ++i) 
            {
                manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
            }
        });
    }

    // Wait for all threads to finish
    for (auto& th : workers)
    {
        th.join();
    }

    // Verify that each service received the correct number of events
    for (auto& svc : services) 
    {
        // Wait for all the events to be dispatched !!
        while (svc->receivedCount.load(std::memory_order_relaxed) < expected)
        {
            if (std::chrono::steady_clock::now() > deadline)
            {
                FAIL() << "Timeout waiting for events";
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        EXPECT_EQ(svc->receivedCount.load(std::memory_order_relaxed), expected);
        svc->unsubscribe(Core::EventType::TestEvent);
    }
}

}// namespace TestStressEventManager
