/**
 * @file test_service_base.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceBaseQueue.hpp"
#include <gtest/gtest.h>
#include <future>
#include <thread>

namespace TestsServiceBaseQueue
{

class TestService : public Core::ServiceBaseQueue
{
public:  
    static constexpr Core::ServiceId staticId = Core::ServiceId::TestService;

    explicit TestService(std::shared_ptr<Core::EventManager> mgr, uint8_t maxEventsPerCycle=16)
        :   ServiceBaseQueue(staticId, std::move(mgr), maxEventsPerCycle),
            count_{0}
    {}

    void run() override
    {
        while (running())
        {
            dispatchLoop();

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void dispatch(const Core::Event& e) override
    {
        (void)e;
        count_.fetch_add(1, std::memory_order_relaxed);
        lastPublisher = e.publisherId();
    }

    int getCount() const
    {
        return count_.load(std::memory_order_relaxed);
    }

    Core::ServiceId lastPublisher{Core::ServiceId::BASE};

private:
    std::atomic<int> count_{0};
};

class TestServiceBlockingDispatch : public Core::ServiceBaseQueue
{
public:  
    static constexpr Core::ServiceId staticId = Core::ServiceId::TestService;

    explicit TestServiceBlockingDispatch(std::shared_ptr<Core::EventManager> mgr, uint8_t maxEventsPerCycle=4)
        :   ServiceBaseQueue(staticId, std::move(mgr), maxEventsPerCycle),
            count_{0}
    {}

    void run() override
    {
        while (running())
        {
            dispatchLoopBlocking();

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void dispatch(const Core::Event& e) override
    {
        (void)e;
        count_.fetch_add(1, std::memory_order_relaxed);
    }

    int getCount() const
    {
        return count_.load(std::memory_order_relaxed);
    }

private:
    std::atomic<int> count_{0};
};

TEST(ServiceBaseQueue, ProcessesEvent)
{
    std::shared_ptr<Core::EventManager> mgr = std::make_shared<Core::EventManager>();
    std::shared_ptr<TestService> svc = std::make_shared<TestService>(mgr);

    ASSERT_NE(svc, nullptr);

    svc->start();
    // std::thread t([&] { svc->run(); });
    std::thread t(&TestService::run, svc);
    
    svc->subscribe(Core::EventType::TestEvent);
    mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    svc->stop();
    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 1);
}

TEST(ServiceBaseQueue, UnsubscribeEvent)
{
    std::shared_ptr<Core::EventManager> mgr = std::make_shared<Core::EventManager>();
    std::shared_ptr<TestService> svc = std::make_shared<TestService>(mgr);

    ASSERT_NE(svc, nullptr);

    svc->start();
    // std::thread t([&] { svc->run(); });
    std::thread t(&TestService::run, svc);
    
    svc->subscribe(Core::EventType::TestEvent);

    mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    ASSERT_EQ(svc->getCount(), 1);

    svc->unsubscribe(Core::EventType::TestEvent);

    mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    svc->stop();
    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 1);
}

TEST(ServiceBaseQueue, ServiceSendEvent)
{
    std::shared_ptr<Core::EventManager> mgr = std::make_shared<Core::EventManager>();
    std::shared_ptr<TestService> svc = std::make_shared<TestService>(mgr);

    ASSERT_NE(svc, nullptr);

    svc->start();
    // std::thread t([&] { svc->run(); });
    std::thread t(&TestService::run, svc);
    
    svc->subscribe(Core::EventType::TestEvent);

    svc->sendEvent(Core::EventType::TestEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    svc->stop();
    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 1);
    EXPECT_EQ(svc->lastPublisher, Core::ServiceId::TestService);
}

TEST(ServiceBaseQueue, TestBlockingDispatch)
{
    auto mgr = std::make_shared<Core::EventManager>();
    auto svc = std::make_shared<TestServiceBlockingDispatch>(mgr, 4);

    svc->start();
    std::thread t(&TestServiceBlockingDispatch::run, svc);

    svc->subscribe(Core::EventType::TestEvent);

    for (int i = 0; i < 10; ++i)
    {
        mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    svc->stop();
    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 10);
}

TEST(ServiceBaseQueue, ProcessesEventsWithPayload)
{
    std::shared_ptr<Core::EventManager> mgr = std::make_shared<Core::EventManager>();
    std::shared_ptr<TestService> svc = std::make_shared<TestService>(mgr);

    ASSERT_NE(svc, nullptr);

    svc->start();
    std::thread t(&TestService::run, svc);
    
    svc->subscribe(Core::EventType::TestEvent);

    Core::Event e(Core::EventType::TestEvent, Core::ServiceId::BASE);

    Core::Event::Payload p;
    p.dataA = 1;
    p.dataB = 2;
    p.dataC = 3;
    p.dataD = 4;
    p.requestId = 99;

    e.setPayload(p);

    mgr->postEvent(e);

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    svc->stop();
    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 1);
}

TEST(ServiceBaseQueue, DropsEventsBeforeStart)
{
    auto mgr = std::make_shared<Core::EventManager>();
    auto svc = std::make_shared<TestService>(mgr);

    // Not calling start() !
    std::thread t(&TestService::run, svc);

    svc->subscribe(Core::EventType::DEFAULT);
    svc->subscribe(Core::EventType::TestEvent);
    mgr->postEvent(Core::Event{Core::EventType::DEFAULT, Core::ServiceId::BASE});
    mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    svc->stop();
    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 0);
}

TEST(ServiceBaseQueue, RespectsBatchLimit)
{
    auto mgr = std::make_shared<Core::EventManager>();
    auto svc = std::make_shared<TestService>(mgr, 15);

    svc->start();
    std::thread t(&TestService::run, svc);

    svc->subscribe(Core::EventType::TestEvent);

    for (int i = 0; i < 20; ++i)
    {
        mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    svc->stop();
    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 20);
}

TEST(ServiceBaseQueue, DrainsQueueOnStop)
{
    auto mgr = std::make_shared<Core::EventManager>();
    auto svc = std::make_shared<TestService>(mgr, 1);

    svc ->start();
    std::thread t(&TestService::run, svc);

    svc->subscribe(Core::EventType::TestEvent);

    ASSERT_EQ(svc->getCount(), 0);

    for (int i = 0; i < 5; ++i)
    {
        mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    svc->stop();

    mgr->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});

    t.join();

    mgr->stop();

    ASSERT_EQ(svc->getCount(), 5);
}

TEST(ServiceBaseQueue, ExpiredSubscriberRemoved) 
{
    auto manager = std::make_shared<Core::EventManager>();
    {
        auto tempService = std::make_shared<TestService>(manager);
        tempService->subscribe(Core::EventType::TestEvent);
        // tempService goes out of scope -> weak_ptr expires
    }

    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    manager->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // No crash, expired subscriber cleaned automatically
    SUCCEED();
}

class TestServiceHook : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::TestService;

    TestServiceHook(std::shared_ptr<Core::EventManager> mgr)
        : ServiceBaseQueue(staticId, mgr)
    {}

    void run() override
    {
        while (running())
        {
            dispatchLoop();
        }
    }

    void dispatch(const Core::Event&) override
    {
        count_.fetch_add(1, std::memory_order_relaxed);
        if (hook_) hook_();
    }

    void setHook(std::function<void()> h)
    {
        hook_ = std::move(h);
    }

    std::atomic<int> count_{0};

private:
    std::function<void()> hook_;
};

TEST(ServiceBaseQueue, DeterministicDispatch)
{
    auto mgr = std::make_shared<Core::EventManager>();
    auto svc = std::make_shared<TestServiceHook>(mgr);

    std::promise<void> processed;
    auto future = processed.get_future();

    svc->setHook([&] {
        processed.set_value();
    });

    svc->subscribe(Core::EventType::TestEvent);
    svc->start();

    // std::thread t([&] { svc->run(); });
    std::thread t(&TestServiceHook::run, svc);

    mgr->postEvent({Core::EventType::TestEvent, svc->id()});

    future.wait();

    svc->stop();
    t.join();

    ASSERT_EQ(svc->count_.load(), 1);
}

} // namespace TestsServiceBaseQueue
