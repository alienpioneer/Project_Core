/**
 * @file test_event_manager.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceBase.hpp"
#include <gtest/gtest.h>
#include <thread>

namespace TestsEventManager
{

class TestDefaultServiceBase : public Core::ServiceBase 
{
public:
    TestDefaultServiceBase(Core::ServiceId id, std::shared_ptr<Core::EventManager> manager)
        : ServiceBase(id, std::move(manager)) 
    {}
};

class TestServiceBase : public Core::ServiceBase 
{
public:
    TestServiceBase(Core::ServiceId id, std::shared_ptr<Core::EventManager> manager)
        : ServiceBase(id, std::move(manager)),
          receivedCount(0) 
    {}

    void onEvent(const Core::Event& event) override 
    {
        lastPublisher = event.publisherId();
        receivedCount.fetch_add(1, std::memory_order_relaxed);
    }

    Core::ServiceId lastPublisher{Core::ServiceId::BASE};
    std::atomic<int> receivedCount{0};
};

TEST(EventManagerTest, TestDefaultServiceBase) 
{
    auto manager = std::make_shared<Core::EventManager>();
    std::shared_ptr<TestDefaultServiceBase> service = std::make_shared<TestDefaultServiceBase>(Core::ServiceId::TestService, manager);

    service->subscribe(Core::EventType::TestEvent);

    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    service->unsubscribe(Core::EventType::TestEvent);

    manager->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    EXPECT_EQ(service->id(), Core::ServiceId::TestService);
}

// EventManager Tests
TEST(EventManagerTest, SingleSubscriberReceivesEvent) 
{
    auto manager = std::make_shared<Core::EventManager>();
    std::shared_ptr<TestServiceBase> service = std::make_shared<TestServiceBase>(Core::ServiceId::TestService, manager);

    service->subscribe(Core::EventType::TestEvent);

    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(service->receivedCount, 1);
    EXPECT_EQ(service->lastPublisher, Core::ServiceId::BASE);

    service->unsubscribe(Core::EventType::TestEvent);

    manager->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(EventManagerTest, MultipleSubscribers) 
{
    auto manager = std::make_shared<Core::EventManager>();
    auto s1 = std::make_shared<TestServiceBase>(Core::ServiceId::TestService, manager);
    auto s2 = std::make_shared<TestServiceBase>(Core::ServiceId::BASE, manager);

    s1->subscribe(Core::EventType::TestEvent);
    s2->subscribe(Core::EventType::TestEvent);

    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(s1->receivedCount.load(), 1);
    EXPECT_EQ(s2->receivedCount.load(), 1);

    s1->unsubscribe(Core::EventType::TestEvent);
    s2->unsubscribe(Core::EventType::TestEvent);

    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(s1->receivedCount.load(), 1);
    EXPECT_EQ(s2->receivedCount.load(), 1);

    manager->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(EventManagerTest, ExpiredSubscriberRemoved) 
{
    auto manager = std::make_shared<Core::EventManager>();
    {
        auto tempService = std::make_shared<TestServiceBase>(Core::ServiceId::TestService, manager);
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

TEST(EventManagerTest, TestUnsubscribe) 
{
    auto manager = std::make_shared<Core::EventManager>();
    auto s = std::make_shared<TestServiceBase>(Core::ServiceId::TestService, manager);

    s->subscribe(Core::EventType::TestEvent);
    s->unsubscribe(Core::EventType::TestEvent);

    manager->postEvent(Core::Event{Core::EventType::TestEvent, Core::ServiceId::BASE});
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(s->receivedCount.load(), 0);

    manager->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(EventManagerTest, SendEventPostsToManager) 
{
    auto manager = std::make_shared<Core::EventManager>();
    auto s1 = std::make_shared<TestServiceBase>(Core::ServiceId::TestService, manager);
    auto s2 = std::make_shared<TestServiceBase>(Core::ServiceId::BASE, manager);

    s2->subscribe(Core::EventType::TestEvent);

    s1->sendEvent(Core::EventType::TestEvent);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    EXPECT_EQ(s2->receivedCount, 1);
    EXPECT_EQ(s2->lastPublisher, Core::ServiceId::TestService);

    s2->unsubscribe(Core::EventType::TestEvent);

    manager->stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

} // namespace TestsEventManager
