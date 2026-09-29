/**
 * @file test_stress_service_manager.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceManager.hpp"
#include <gtest/gtest.h>
#include <vector>

namespace TestStressServiceManager
{

class CountingService : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::TestService;

    CountingService(std::shared_ptr<Core::EventManager> mgr, uint8_t maxEventsPerCycle=15)
        :   ServiceBaseQueue(staticId, std::move(mgr), maxEventsPerCycle)
    {}

    void init() noexcept override
    {
        subscribe(Core::EventType::TestEvent);
    }

    void dispatch(const Core::Event& /*e*/) override
    {
        count_.fetch_add(1, std::memory_order_relaxed);
    }

    int getCount() const
    {
        return count_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> count_{0};
};

TEST(ServiceManager, StressHighLoad)
{
    auto mgr = std::make_shared<Core::EventManager>();
    Core::ServiceManager sm(mgr);

    sm.registerService<CountingService>();

    constexpr int threadCount = 4;
    constexpr int eventsPerThread = 10000;

    std::vector<std::thread> producers;

    for (int t = 0; t < threadCount; ++t)
    {
        producers.emplace_back([&] {
            for (int i = 0; i < eventsPerThread; ++i)
            {
                mgr->postEvent({Core::EventType::TestEvent,Core::ServiceId::BASE});
            }
        });
    }

    for (auto& p : producers)
        p.join();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    sm.stopAll();

    auto total = sm.get<CountingService>()->getCount();
    ASSERT_GT(total, 0);
}

TEST(ServiceManager, StressStartStopCycles)
{
    for (int i = 0; i < 50; ++i)
    {
        auto mgr = std::make_shared<Core::EventManager>();
        Core::ServiceManager sm(mgr);

        sm.registerService<CountingService>();

        mgr->postEvent({Core::EventType::TestEvent,Core::ServiceId::BASE});

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        sm.stopAll();
    }

    SUCCEED();
}

} // namespace TestStressServiceManager
