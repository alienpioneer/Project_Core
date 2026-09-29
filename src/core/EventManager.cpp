/**
 * @file EventManager.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#include "core/EventManager.hpp"
#include "core/AQueue.hpp"
#include "core/AVector.hpp"
#include "Config.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>

// #include <iostream>

namespace Core
{

struct EventManager::Impl
{
    std::array<AVector<std::weak_ptr<ISubscriber>, MAX_SERVICES>,MAX_EVENTS> subscribers_;

    AQueue<Event, EVENTS_QUEUE_MAX_SIZE > eventQueue_;

    std::mutex subscribersMutex_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    std::atomic<bool> running_{true};
    std::thread dispatcherThread_;

    // The dispatch loop should run only inside the worker thread
    void dispatchLoop();

    // Returns the next event from the queue - to be used with the dispatchLoop()
    Event waitAndPop();

    // Dispatch a single event from the queue to the subscribers
    void dispatch(const Event& event);
};

EventManager::EventManager() 
    : impl_(std::make_unique<Impl>())
{
    // RAII Aquire and start dispatcher thread in the constructor
    impl_->dispatcherThread_ = std::thread(&EventManager::Impl::dispatchLoop, impl_.get() );
}

EventManager::~EventManager()
{
    stop();

    // RAII Join dispatcher thread in the destructor
    if (impl_->dispatcherThread_.joinable())
    {
        impl_->dispatcherThread_.join();
    }

    assert(!impl_->dispatcherThread_.joinable());
}

void EventManager::subscribe(EventType type, const std::shared_ptr<ISubscriber>& subscriber)
{
    std::lock_guard<std::mutex> lock(impl_->subscribersMutex_);

    impl_->subscribers_[static_cast<std::size_t>(type)].push_back(subscriber);
}

void EventManager::unsubscribe(EventType type, const ISubscriber* subscriber)
{
    std::lock_guard<std::mutex> lock(impl_->subscribersMutex_);

    auto& subVector = impl_->subscribers_[static_cast<std::size_t>(type)];

    for (auto it = subVector.begin(); it != subVector.end(); )
    {
        auto sub = it->lock();
        
        if (!sub || sub.get() == subscriber)
        {
            it = subVector.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EventManager::postEvent(Event event)
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

/**
 * @brief The dispatch loop should run only inside the worker thread
 * 
 */
void EventManager::Impl::dispatchLoop() 
{
    while (true)
    {
        Event event = waitAndPop();

        if (event.type() == EventType::SHUTDOWN)
        {
            dispatch(event);
            // std::cout << "==>> EVENT MANAGER SHUTDOWN " << std::endl;
            break;
        }
            
        dispatch(event);
    }
}

/**
 * @brief Returns the next event from the queue - to be used with the dispatchLoop()
 * 
 * @return Event 
 */
Event EventManager::Impl::waitAndPop()
{
    std::unique_lock<std::mutex> lock(queueMutex_);

    queueCV_.wait(lock, [this] { return (!eventQueue_.empty() || !running_); });

    if (!running_ && eventQueue_.empty())
    {
        return Event(EventType::SHUTDOWN, ServiceId::BASE);
    }
        
    Event event = std::move(eventQueue_.get());
    eventQueue_.pop();
    return event;
}

/**
 * @brief Dispatch a single event from the queue to the subscribers
 * 
 * @param event 
 */
void EventManager::Impl::dispatch(const Event& event) 
{
    // Keep a subscribers snapshot to avoid calling onEvent() under lock to avoid deadlocks!
    AVector<std::shared_ptr<ISubscriber>, Core::MAX_SERVICES> snapshot;
    const std::size_t index = static_cast<std::size_t>(event.type());

    assert(index < subscribers_.size());

    {
        std::lock_guard<std::mutex> lock(subscribersMutex_);

        auto& subscribersList = subscribers_[index];

        if (index >= subscribers_.size())
        {
            return;
        }

        if (subscribersList.empty())
        {
            // If no subscribers return
            return;
        }

        for (auto it = subscribersList.begin(); it != subscribersList.end(); )
        {
            if (auto sub = it->lock())
            {
                snapshot.push_back(sub);
                ++it;
            }
            else
            {
                it = subscribersList .erase(it); // safe
            }
        }
    }

    for (auto& sub : snapshot) 
    {
        sub->onEvent(event);  // safe, EventManager guaranteed alive
    }
}

void EventManager::stop() noexcept
{ 
    impl_->running_ = false;
    impl_->queueCV_.notify_all();
}


}// namespace Core
