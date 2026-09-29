/**
 * @file test_aqueue.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/AQueue.hpp"
#include "core/Event.hpp"
#include "ServiceTypes.hpp"
#include <gtest/gtest.h>

namespace TestAQueue
{

TEST(AQueue, EmptyOnConstruction)
{
    Core::AQueue<int, 4> q;
    EXPECT_TRUE(q.empty());
    EXPECT_FALSE(q.full());
}

TEST(AQueue, PopOnEmptyFails)
{
    Core::AQueue<int, 4> q;
    EXPECT_FALSE(q.pop());
}

//Push / Get / Pop basic behavior
TEST(AQueue, PushGetPopSingle)
{
    Core::AQueue<int, 4> q;

    ASSERT_TRUE(q.push(42));
    EXPECT_FALSE(q.empty());

    EXPECT_EQ(q.get(), 42);
    EXPECT_TRUE(q.pop());
    EXPECT_TRUE(q.empty());
}

//FIFO ordering
TEST(AQueue, FIFOOrder)
{
    Core::AQueue<int, 4> q;

    ASSERT_TRUE(q.push(1));
    ASSERT_TRUE(q.push(2));
    ASSERT_TRUE(q.push(3));

    EXPECT_EQ(q.get(), 1);
    q.pop();

    EXPECT_EQ(q.get(), 2);
    q.pop();

    EXPECT_EQ(q.get(), 3);
    q.pop();

    EXPECT_TRUE(q.empty());
}

// Full queue behavior
TEST(AQueue, FullQueue)
{
    Core::AQueue<int, 3> q;

    EXPECT_TRUE(q.push(1));
    EXPECT_TRUE(q.push(2));
    EXPECT_TRUE(q.push(3));

    EXPECT_TRUE(q.full());
    EXPECT_FALSE(q.push(4)); // overflow rejected
}

// Full queue behavior
TEST(AQueue, QueueSize)
{
    Core::AQueue<int, 10> q;

    EXPECT_TRUE(q.push(1));
    EXPECT_TRUE(q.push(2));
    EXPECT_TRUE(q.push(3));

    EXPECT_EQ(q.size(), 3);
}

// Wrap-around correctness
TEST(AQueue, WrapAround)
{
    Core::AQueue<int, 3> q;

    q.push(1);
    q.push(2);
    q.push(3);

    EXPECT_EQ(q.get(), 1);
    q.pop();

    EXPECT_TRUE(q.push(4));

    EXPECT_EQ(q.get(), 2);
    q.pop();

    EXPECT_EQ(q.get(), 3);
    q.pop();

    EXPECT_EQ(q.get(), 4);
    q.pop();

    EXPECT_TRUE(q.empty());
}

// Const correctness
TEST(AQueue, ConstGet)
{
    Core::AQueue<int, 2> q;
    q.push(7);

    const auto& cq = q;
    EXPECT_EQ(cq.get(), 7);
}

TEST(AQueueEvent, EventPushAndGet)
{
    Core::AQueue<Core::Event, 4> q;

    ASSERT_TRUE(q.push({Core::EventType::SHUTDOWN, Core::ServiceId::TestService}));
    ASSERT_FALSE(q.empty());

    const auto& e = q.get();
    EXPECT_EQ(e.type(), Core::EventType::SHUTDOWN);
    EXPECT_EQ(e.publisherId(), Core::ServiceId::TestService);
}

TEST(AQueueEvent, MovePushAndGet)
{
    Core::AQueue<Core::Event, 4> q;

    ASSERT_TRUE(q.push({Core::EventType::SHUTDOWN, Core::ServiceId::TestService}));
    ASSERT_FALSE(q.empty());
    Core::Event e = Core::Event(Core::EventType::SHUTDOWN, Core::ServiceId::TestService);
    ASSERT_TRUE(q.push(e));

    e = q.get();
    EXPECT_EQ(e.type(), Core::EventType::SHUTDOWN);
    EXPECT_EQ(e.publisherId(), Core::ServiceId::TestService);
    q.pop();

    e = q.get();
    EXPECT_EQ(e.type(), Core::EventType::SHUTDOWN);
    EXPECT_EQ(e.publisherId(), Core::ServiceId::TestService);
}

}// namespace TestAQueue
