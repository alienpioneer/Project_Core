/**
 * @file ServiceBaseQueue.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/ServiceBaseQueue.hpp"
#include "core/AQueue.hpp"
#include "Config.hpp"
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <iostream>

namespace Core
{

struct ServiceBaseQueue::Impl
{
    ServiceId id_;
    std::shared_ptr<EventManager> manager_;
    AQueue<Event, SERVICE_QUEUE_MAX_SIZE > eventQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::atomic<bool> running_{false};   
};
 
// 
/**
 * @brief Construct a new Service Base Queue:: Service Base Queue object. 
 * Do NOT call shared_from_this() in this constructor !
 * 
 * @param id 
 * @param manager 
 * @param maxEventsPerCycle 
 */
ServiceBaseQueue::ServiceBaseQueue(ServiceId id, std::shared_ptr<EventManager> manager, uint8_t maxEventsPerCycle)
    :   impl_(std::make_unique<Impl>()),
        maxEventsPerCycle_{maxEventsPerCycle}
{
    impl_->id_ = id;
    impl_->manager_ = std::move(manager);
    impl_->running_ = false;
}

ServiceBaseQueue::~ServiceBaseQueue() noexcept = default;

void ServiceBaseQueue::subscribe(EventType type) 
{
    impl_->manager_->subscribe(type, shared_from_this());
}

void ServiceBaseQueue::unsubscribe(EventType type) 
{
    impl_->manager_->unsubscribe(type, this);
}

void ServiceBaseQueue::sendEvent(EventType type)
{
    impl_->manager_->postEvent(Event{type, impl_->id_});
}

void ServiceBaseQueue::sendEvent(Event e)
{
    e.resetId(impl_->id_);
    impl_->manager_->postEvent(e);
}

void ServiceBaseQueue::onEvent(const Event& event)
{
    if (!impl_->running_)
    {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(impl_->queueMutex_);
        impl_->eventQueue_.push(std::move(event));
    }

    impl_->queueCV_.notify_one();
}

void ServiceBaseQueue::dispatchLoop()
{
    if (impl_->eventQueue_.empty() || !impl_->running_)
    {
        return;
    }
    
    uint8_t processed = 0;

    while (processed < maxEventsPerCycle_)
    {
        Event event;

        {
            std::lock_guard<std::mutex> lock(impl_->queueMutex_);

            if (impl_->eventQueue_.empty())
                return;

            event = std::move(impl_->eventQueue_.get());
            impl_->eventQueue_.pop();
        }

        dispatch(event);
        ++processed;
    }
}

void ServiceBaseQueue::dispatchLoopBlocking()
{
    uint8_t processed = 0;

    while (processed < maxEventsPerCycle_)
    {
        Event event;

        {
            std::unique_lock<std::mutex> lock(impl_->queueMutex_);

            if (processed == 0)
            {
                impl_->queueCV_.wait(lock, [this] { return (!impl_->eventQueue_.empty() || !impl_->running_); });
            }

            // Gracefull shutdown
            if (!impl_->running_ && impl_->eventQueue_.empty())
            {
                return;
            }
            
            if (impl_->eventQueue_.empty())
            {
                return;
            }
                
            event = std::move(impl_->eventQueue_.get());

            impl_->eventQueue_.pop();
        }
        
        dispatch(event);

        processed++;
    }
}

void ServiceBaseQueue::run()
{
    setup();

    while(impl_->running_)
    {
        update();

        dispatchLoop();

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void ServiceBaseQueue::stop() noexcept
{ 
    impl_->running_ = false;
    impl_->queueCV_.notify_all();
}

void ServiceBaseQueue::start() noexcept
{ 
    impl_->running_ = true;
}

ServiceId ServiceBaseQueue::id() const noexcept
{ 
    return impl_->id_;
}

bool ServiceBaseQueue::running() const noexcept
{
    return impl_->running_.load();
}

}// namespace Core