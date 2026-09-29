/**
 * @file test_event.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/Event.hpp"

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>

namespace TestsEvent
{

using namespace Core;

TEST(EventTest, DefaultConstruction)
{
    Event e;

    EXPECT_EQ(e.type(), EventType::DEFAULT);
    EXPECT_EQ(e.publisherId(), ServiceId::BASE);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(e.payload()));
}

TEST(Event, ConstructWithTypeAndPublisher)
{
    Event e(EventType::TestEvent, ServiceId::TestService);

    EXPECT_EQ(e.type(), EventType::TestEvent);
    EXPECT_EQ(e.publisherId(), ServiceId::TestService);
}

TEST(EventTest, ConstructionWithTypeAndPublisher)
{
    Event e(EventType::TestEvent, ServiceId::TestService);

    EXPECT_EQ(e.type(), EventType::TestEvent);
    EXPECT_EQ(e.publisherId(), ServiceId::TestService);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(e.payload()));
}

TEST(Event, ConstructWithTypeOnly)
{
    Event e(EventType::SHUTDOWN);

    EXPECT_EQ(e.type(), EventType::SHUTDOWN);
    EXPECT_EQ(e.publisherId(), ServiceId::BASE); // default
}

// Default construction behavior
TEST(Event, DefaultConstructionPublisherId)
{
    Event e;
    EXPECT_EQ(e.publisherId(), ServiceId::BASE);
    EXPECT_EQ(e.type(), EventType::DEFAULT);
}

TEST(Event, ResetPublisherId)
{
    Event e;
    EXPECT_EQ(e.publisherId(), ServiceId::BASE);
    EXPECT_EQ(e.type(), EventType::DEFAULT);
    e.resetId(ServiceId::TestService);
    EXPECT_EQ(e.publisherId(), ServiceId::TestService);
}

class MockTestSubscriber final : public ISubscriber
{
public:
    void onEvent(const Event& event) override
    {
        lastType = event.type();
        lastPublisher = event.publisherId();
        called = true;
    }

    bool called{false};
    EventType lastType{};
    ServiceId lastPublisher{ServiceId::BASE};
};

TEST(ISubscriber, ReceivesEvent)
{
    MockTestSubscriber sub;
    Event e(EventType::SHUTDOWN, ServiceId::TestService);

    sub.onEvent(e);

    EXPECT_TRUE(sub.called);
    EXPECT_EQ(sub.lastType, EventType::SHUTDOWN);
    EXPECT_EQ(sub.lastPublisher, ServiceId::TestService);
}

TEST(EventTest, SetIntPayload)
{
    Event e(EventType::TestEvent);

    e.setPayload(42);

    ASSERT_TRUE(std::holds_alternative<int>(e.payload()));
    EXPECT_EQ(std::get<int>(e.payload()), 42);
}

TEST(EventTest, GetConstPayload)
{
    Event e(EventType::TestEvent);

    e.setPayload(32);
    const int* payld = std::get_if<int>(&e.payload());

    EXPECT_EQ(*payld, 32);
}

TEST(EventTest, SetLvalPayload)
{
    Event e(EventType::TestEvent);

    e.setPayload({32, 33, 34, 35, 1});
    const Event::Payload* payld = std::get_if<Event::Payload>(&e.payload());

    EXPECT_EQ(payld->dataA, 32);
    EXPECT_EQ(payld->dataB, 33);
    EXPECT_EQ(payld->dataC, 34);
    EXPECT_EQ(payld->dataD, 35);
    EXPECT_EQ(payld->requestId, 1);
}

TEST(EventTest, SetStructuredPayload)
{
    Event e(EventType::TestEvent);

    Event::Payload p;
    p.dataA = 1;
    p.dataB = 2;
    p.dataC = 3;
    p.dataD = 4;
    p.requestId = 99;

    e.setPayload(p);

    ASSERT_TRUE(std::holds_alternative<Event::Payload>(e.payload()));

    const auto& stored = std::get<Event::Payload>(e.payload());
    EXPECT_EQ(stored.dataA, 1);
    EXPECT_EQ(stored.dataB, 2);
    EXPECT_EQ(stored.dataC, 3);
    EXPECT_EQ(stored.dataD, 4);
    EXPECT_EQ(stored.requestId, 99);
}

TEST(EventTest, PayloadTypeOverwrite)
{
    Event e(EventType::TestEvent);

    e.setPayload(7);
    ASSERT_TRUE(std::holds_alternative<int>(e.payload()));

    Event::Payload p;
    p.dataA = 10;
    e.setPayload(p);

    ASSERT_TRUE(std::holds_alternative<Event::Payload>(e.payload()));
}

TEST(EventTest, MoveEventPreservesPayload)
{
    Event e(EventType::TestEvent);
    e.setPayload(123);

    Event moved(std::move(e));

    ASSERT_TRUE(std::holds_alternative<int>(moved.payload()));
    EXPECT_EQ(std::get<int>(moved.payload()), 123);
}

TEST(EventTest, SafeAccessUsingGetIf)
{
    Event e(EventType::TestEvent);
    e.setPayload(55);

    const auto* id = std::get_if<int>(&e.payload());
    ASSERT_NE(id, nullptr);
    EXPECT_EQ(*id, 55);

    const auto* p = std::get_if<Event::Payload>(&e.payload());
    EXPECT_EQ(p, nullptr);
}


TEST(EventTest, SafeAccessString)
{
    Event e(EventType::TestEvent);
    e.setPayload("gtest");

    const auto* str = std::get_if<std::string>(&e.payload());
    ASSERT_NE(str, nullptr);
    EXPECT_EQ(*str, "gtest");
}

}// namespace TestsEvent
