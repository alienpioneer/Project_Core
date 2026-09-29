/**
 * @file main.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include <iostream>
#include <cassert>
#include "core/ATimer.hpp"
#include "core/ServiceBase.hpp"
#include "core/ServiceBaseQueue.hpp"

constexpr std::size_t NUM_SUBSCRIBERS = 8;
constexpr std::size_t NUM_MESSAGES    = 100000;
constexpr int NUM_PRODUCERS  = 8;
constexpr int MESSAGES_PER_PRODUCER = 10000;

class MyClass
{
public:
    void printMessage() {
        std::cout << "Member function called!\n";
    }
};

class TestServiceSimple : public Core::ServiceBase 
{
public:
    TestServiceSimple(Core::ServiceId id, const std::shared_ptr<Core::EventManager>& mgr, std::atomic<std::size_t>& counter)
        : ServiceBase(id, mgr),
          counter_(counter)
    {}

    void onEvent(const Core::Event& event) override
    {
        if (event.type() == Core::EventType::TestEvent)
        {
            counter_.fetch_add(1, std::memory_order_relaxed);
            // ++counter_;
        }
            
    }

    int counter() const
    {
        return static_cast<int>(counter_);
    }

private:
    std::atomic<std::size_t>& counter_;
};

class TestService : public Core::ServiceBaseQueue
{
public:
    explicit TestService(Core::ServiceId id, std::shared_ptr<Core::EventManager> mgr)
        : ServiceBaseQueue(id, std::move(mgr))
    {}

    void run() override
    {
        while (running())
        {
            dispatchLoop();
        }
    }

    void dispatch(const Core::Event& e) override
    {
        (void)e;
        count_.fetch_add(1, std::memory_order_relaxed);
    }

    std::atomic<int> count_{0};
};

void regularFunction() 
{
    std::cout << "Regular function called\n";
}

void testTimer()
{
    // One-shot timer (default)
    Core::ATimer timer1(std::chrono::seconds(1), []() {
        std::cout << "One-shot timer fired!\n";
        });
    timer1.start();

    // Repeating timer
    int count = 0;
    Core::ATimer timer2(std::chrono::milliseconds(500), [&count]() {
        std::cout << "Repeating timer: " << ++count << "\n";
        }, false); // false = repeating
    timer2.start();

    std::this_thread::sleep_for(std::chrono::milliseconds(2500));

    // Stop repeating timer
    timer2.stop();
    std::cout << "Stopped repeating timer\n";

    // Restart it
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    timer2.start();
    std::cout << "Restarted repeating timer\n";

    std::this_thread::sleep_for(std::chrono::seconds(2));
    timer2.stop();

    // Timer with member function
    MyClass obj;
    Core::ATimer timer3(std::chrono::milliseconds(800), [&obj]() {
        obj.printMessage();
        });
    timer3.start();

    // Timer with regular function
    Core::ATimer timer4(std::chrono::milliseconds(600), regularFunction);
    timer4.start();

    std::this_thread::sleep_for(std::chrono::seconds(2));
}

void testTimerConcurrentStartStop()
{
    int counter = 0;

    // Create repeating timer
    Core::ATimer timer(std::chrono::milliseconds(200), [&counter]() {
        std::cout << "Timer fired: " << ++counter << std::endl;
        }, false);

    // Start from main thread
    timer.start();
    std::cout << "Timer started from main thread" << std::endl;

    // Control thread that stops/starts the timer
    std::thread controlThread([&timer]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Stopping timer from control thread" << std::endl;
        timer.stop();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Restarting timer from control thread" << std::endl;
        timer.start();

        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Stopping timer again from control thread" << std::endl;
        timer.stop();
        });

    // Let it run
    controlThread.join();

    std::cout << "Test completed. Final counter: " << counter << std::endl;
}

void eventsBasicTest()
{
    std::shared_ptr<Core::EventManager> manager = std::make_shared<Core::EventManager>();

    auto s1 = std::make_shared<Core::ServiceBase>(Core::ServiceId::BASE, manager);
    auto s2 = std::make_shared<Core::ServiceBase>(Core::ServiceId::TestService, manager);

    s1->subscribe(Core::EventType::TestEvent);
    s2->subscribe(Core::EventType::TestEvent);
    s2->subscribe(Core::EventType::DEFAULT);

    s1->sendEvent(Core::EventType::TestEvent);
    s2->sendEvent(Core::EventType::DEFAULT);

    // Give some time for dispatcher to process
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    s1->sendEvent(Core::EventType::SHUTDOWN);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void servicesRobustnessTest()
{
    auto manager = std::make_shared<Core::EventManager>();
    std::atomic<std::size_t> receivedCount{0};
    std::vector<std::shared_ptr<TestServiceSimple>> services;

    for (std::size_t i = 0; i < NUM_SUBSCRIBERS; ++i)
    {
        auto s = std::make_shared<TestServiceSimple>(Core::ServiceId::TestService, manager, receivedCount);

        s->subscribe(Core::EventType::TestEvent);

        services.push_back(s);
    }

    // Send N events concurrently
    std::thread producer([&] 
        {
            std::atomic<std::size_t> sendCounter{0};
            std::shared_ptr<TestServiceSimple> sender = std::make_shared<TestServiceSimple>(Core::ServiceId::TestService, manager, sendCounter);
            sender->subscribe(Core::EventType::TestEvent);

            for (std::size_t i = 0; i < NUM_MESSAGES; ++i)
            {
                // manager->postEvent(Core::Event{Core::EventType::DATA_AVAILABLE, Core::ServiceId::BASE});
                sender->sendEvent(Core::EventType::TestEvent);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(300));

            std::cout << "Sent " << sender->counter() << " events from sender."<< std::endl;
        });

    producer.join();

    // Wait until all messages are dispatched
    const std::size_t expected = NUM_MESSAGES * NUM_SUBSCRIBERS;

    while (receivedCount.load(std::memory_order_relaxed) < expected) 
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "ReceivedCount " << receivedCount << " events" << std::endl;
    assert(receivedCount == expected);
    std::cout << "Test passed for " << receivedCount << " events" << std::endl;
}

void multiProducersRobustnessTest()
{
    auto manager = std::make_shared<Core::EventManager>();
    std::atomic<std::size_t> receivedCount{0};
    std::vector<std::shared_ptr<TestServiceSimple>> services;

    for (std::size_t i = 0; i < NUM_SUBSCRIBERS; ++i)
    {
        auto s = std::make_shared<TestServiceSimple>(Core::ServiceId::TestService, manager, receivedCount);

        s->subscribe(Core::EventType::TestEvent);

        services.push_back(s);
    }

    std::vector<std::thread> producersThreads;

    for (int p = 0; p < NUM_PRODUCERS; ++p)
    {   
        producersThreads.emplace_back([manager, p]()
            {
                std::atomic<std::size_t> sendCounter{0};
                std::shared_ptr<TestServiceSimple> sender = std::make_shared<TestServiceSimple>(Core::ServiceId::TestService, manager, sendCounter);
                sender->subscribe(Core::EventType::TestEvent);

                for (int i = 0; i < MESSAGES_PER_PRODUCER; ++i)
                {
                    sender->sendEvent(Core::EventType::TestEvent);
                    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << "Producer " << p << " sent " << sender->counter() << " events."<< std::endl;
            });
    }

    for (auto& t : producersThreads)
    {
        t.join();
    }

    const int EXPECTED = NUM_SUBSCRIBERS * NUM_PRODUCERS * MESSAGES_PER_PRODUCER;

    while (receivedCount.load(std::memory_order_relaxed) < EXPECTED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    assert(receivedCount == EXPECTED);
    std::cout << "Test multi producers passed for " << receivedCount << " events" << std::endl;
}

void simpleServiceTest()
{
    std::shared_ptr<Core::EventManager> mgr = std::make_shared<Core::EventManager>();
    std::shared_ptr<TestService> ui_svc = std::make_shared<TestService>(Core::ServiceId::TestService, mgr);
    std::shared_ptr<TestService> can_svc = std::make_shared<TestService>(Core::ServiceId::BASE, mgr);

    ui_svc->start();
    std::thread t1([&] { ui_svc->run(); });
    can_svc->start();
    std::thread t2([&] { can_svc->run(); });
    
    ui_svc->subscribe(Core::EventType::TestEvent);

    for (int i = 0; i < 999; ++i)
    {
        can_svc->sendEvent(Core::EventType::TestEvent);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ui_svc->stop();
    can_svc->stop();
    t1.join();
    t2.join();

    std::cout << "Received " << ui_svc->count_.load() << " events." << std::endl;
}

int main()
{
    testTimer();

    testTimerConcurrentStartStop();

    eventsBasicTest();

    servicesRobustnessTest();
    
    multiProducersRobustnessTest();

    simpleServiceTest();

    return 0;
}