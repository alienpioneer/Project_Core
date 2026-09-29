/**
 * @file test_service_manager.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceManager.hpp"
#include <gtest/gtest.h>

namespace TestsServiceManager
{

class CountingService : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::TestService;

    CountingService(std::shared_ptr<Core::EventManager> mgr, uint8_t maxEventsPerCycle=15)
        :   ServiceBaseQueue(staticId, std::move(mgr)),
            maxEventsPerCycle_(maxEventsPerCycle)
    {}

    void init() noexcept override
    {
        subscribe(Core::EventType::TestEvent);
        subscribe(Core::EventType::DEFAULT);
    }

    void dispatch(const Core::Event& e) override
    {
        if(e.type() == Core::EventType::DEFAULT)
        {
            received_count_.fetch_add(1, std::memory_order_relaxed);
        }
        else
        {
            count_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    int getCount() const
    {
        return count_.load(std::memory_order_relaxed);
    }

    int getReceivedCount() const
    {
        return received_count_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> count_{0};
    std::atomic<int> received_count_{0};
    uint8_t maxEventsPerCycle_{1};
};

class CountingService2 : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::BASE;

    CountingService2(std::shared_ptr<Core::EventManager> mgr, uint8_t maxEventsPerCycle=15)
        :   ServiceBaseQueue(staticId, std::move(mgr)),
            maxEventsPerCycle_(maxEventsPerCycle)
    {}

    void init() noexcept override
    {
        subscribe(Core::EventType::TestEvent);
    }

    void dispatch(const Core::Event& e) override
    {
        if(e.type() == Core::EventType::TestEvent)
        {
            // Used just for tests
            sendEvent(Core::EventType::DEFAULT);
        }

        count_.fetch_add(1, std::memory_order_relaxed);
    }

    int getCount() const
    {
        return count_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> count_{0};
    uint8_t maxEventsPerCycle_{1};
};

TEST(ServiceManager, StartsAndProcesses)
{
    auto mgr = std::make_shared<Core::EventManager>();
    Core::ServiceManager serviceManager(mgr);

    serviceManager.registerService<CountingService>();
    serviceManager.registerService<CountingService2>();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto svc = serviceManager.get<CountingService>();
    auto svc2 = serviceManager.get<CountingService2>();

    ASSERT_EQ(svc->getCount(), 1);
    ASSERT_EQ(svc->getReceivedCount(), 1);
    ASSERT_EQ(svc2->getCount(), 1);

    mgr->stop();
    serviceManager.stopAll();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(ServiceManager, StopAllStopsServices)
{
    auto mgr = std::make_shared<Core::EventManager>();
    Core::ServiceManager sm(mgr);

    sm.registerService<CountingService>();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    mgr->stop();
    sm.stopAll();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    auto svc = sm.get<CountingService>();
    ASSERT_FALSE(svc->getCount());
}

TEST(ServiceManager, MultipleServicesIndependent)
{
    auto mgr = std::make_shared<Core::EventManager>();
    Core::ServiceManager sm(mgr);

    sm.registerService<CountingService>();
    sm.registerService<CountingService2>();

    for (int i = 0; i < 10; ++i)
    {
        mgr->postEvent({Core::EventType::TestEvent, Core::ServiceId::BASE});
        mgr->postEvent({Core::EventType::DEFAULT, Core::ServiceId::BASE});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    ASSERT_EQ(sm.get<CountingService>()->getCount(), 10);
    ASSERT_EQ(sm.get<CountingService>()->getReceivedCount(), 20);
    ASSERT_EQ(sm.get<CountingService2>()->getCount(), 10);

    sm.stopAll();
}

TEST(ServiceManager, InterServiceCommunication)
{
    auto mgr = std::make_shared<Core::EventManager>();
    Core::ServiceManager sm(mgr);

    sm.registerService<CountingService>();
    sm.registerService<CountingService2>();

    mgr->postEvent({Core::EventType::TestEvent, Core::ServiceId::BASE});

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ASSERT_EQ(sm.get<CountingService>()->getCount(), 1);
    ASSERT_EQ(sm.get<CountingService>()->getReceivedCount(), 1);
    ASSERT_EQ(sm.get<CountingService2>()->getCount(), 1);

    sm.stopAll();
}

}// namespace TestsServiceManager

