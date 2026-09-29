/**
 * @file test_stress_service_payload.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceManager.hpp"
#include "core/ServiceBaseQueue.hpp"
#include "core/EventManager.hpp"
#include "core/Event.hpp"
#include "Config.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <mutex>

namespace TestStressServicePayload
{

static constexpr int multiplier = 2;

static constexpr int maxIterations = Core::MAX_SERVICES * multiplier;
static constexpr int highLoadMaxIterations = Core::MAX_SERVICES * multiplier * 2;

class TestServiceA final : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::BASE;

    explicit TestServiceA(std::shared_ptr<Core::EventManager> mgr)
        : Core::ServiceBaseQueue(staticId, mgr)
    {}

    void init() noexcept override
    {
        subscribe(Core::EventType::TestEvent);

        // kickstart
        Core::Event::Payload p;
        p.dataA = 1;
        p.requestId = 1;

        Core::Event e(Core::EventType::TestEvent, id());
        e.setPayload(p);
        sendEvent(e);

    }

    void dispatch(const Core::Event& event) override
    {
        if (auto* p = std::get_if<Core::Event::Payload>(&event.payload()))
        {
            if (p->requestId >= maxIterations)
                return;

            Core::Event::Payload reply;
            reply.dataA = p->dataA + 1;
            reply.requestId = p->requestId + 1;

            Core::Event e(Core::EventType::TestEvent, id());
            e.setPayload(reply);
            sendEvent(e);
        }
    }
};

class TestServiceB final : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::TestService;

    explicit TestServiceB(std::shared_ptr<Core::EventManager> mgr)
        : Core::ServiceBaseQueue(staticId, mgr)
    {}

    void init() noexcept override
    {
        subscribe(Core::EventType::TestEvent);
    }

    void dispatch(const Core::Event& event) override
    {
        if (auto* p = std::get_if<Core::Event::Payload>(&event.payload()))
        {
            std::lock_guard<std::mutex> lock(mutex_);
            lastValue = p->dataA;
            lastRequest = p->requestId;
            ++count;
        }
    }

    std::atomic<int> count{0};
    int lastValue{0};
    int lastRequest{0};
    std::mutex mutex_;
};

class MixedSenderService final : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::BASE;

    MixedSenderService(std::shared_ptr<Core::EventManager> mgr)
        : Core::ServiceBaseQueue(staticId, mgr)
    {}

    void init() noexcept override {}

    void run() override
    {
        for (int i = 0; i < highLoadMaxIterations; ++i)
        {
            Core::Event e(Core::EventType::TestEvent, id());

            if (i % 3 == 0)
                e.setPayload(i);
            else if (i % 3 == 1)
                e.setPayload(std::string_view("abc"));
            else
            {
                Core::Event::Payload p;
                p.dataA = i;
                p.requestId = i;
                e.setPayload(p);
            }

            sendEvent(e);
        }

        while (running())
        {
            dispatchLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void dispatch(const Core::Event&) override {}
};

class MixedReceiverService final : public Core::ServiceBaseQueue
{
public:
    static constexpr Core::ServiceId staticId = Core::ServiceId::TestService;

    MixedReceiverService(std::shared_ptr<Core::EventManager> mgr)
        : Core::ServiceBaseQueue(staticId, mgr)
    {}

    void init() noexcept override
    {
        subscribe(Core::EventType::TestEvent);
    }

    void dispatch(const Core::Event& event) override
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (std::holds_alternative<int>(event.payload()))
            ++intCount;
        else if (std::holds_alternative<std::string_view>(event.payload()))
            ++stringCount;
        else if (std::holds_alternative<Core::Event::Payload>(event.payload()))
            ++structCount;

        total.fetch_add(1, std::memory_order::memory_order_relaxed);
    }

    std::atomic<int> total{0};
    int intCount{0};
    int stringCount{0};
    int structCount{0};
    std::mutex mutex_;
};

TEST(ServicePayloadStress, BidirectionalStructPingPong)
{
    auto eventMgr = std::make_shared<Core::EventManager>();
    Core::ServiceManager sm(eventMgr);

    sm.registerService<TestServiceA>();
    sm.registerService<TestServiceB>();

    auto svcB = sm.get<TestServiceB>();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    EXPECT_GE(svcB->count.load(), Core::MAX_SERVICES);

    eventMgr->stop();
    sm.stopAll();
}

TEST(ServicePayloadStress, MixedVariantHighLoad)
{
    auto eventMgr = std::make_shared<Core::EventManager>();
    Core::ServiceManager sm(eventMgr);

    sm.registerService<MixedSenderService>();
    sm.registerService<MixedReceiverService>();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    auto receiver = sm.get<MixedReceiverService>();

    EXPECT_EQ(receiver->total.load(), highLoadMaxIterations);

    eventMgr->stop();
    sm.stopAll();
}

}
