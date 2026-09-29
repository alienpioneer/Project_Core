/**
 * @file ATimer.cpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 *
 */

#include "core/ATimer.hpp"

namespace Core
{

void ATimer::run()
{
	Callback callbackCopy = nullptr;

	auto next_fire = std::chrono::steady_clock::now();

    while (running_)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);

			next_fire += delay_;

			if (control_variable_.wait_until(lock, next_fire, [this]{ return !running_;}))
			{
				break;
			}

			// Copy callback to release the callback_ during execution 
			callbackCopy = callback_;
		}

		if (callbackCopy)
		{
			callbackCopy();
		}

		if (one_shot_)
		{
			running_ = false;
			break;
		}
	}
}

void ATimer::start()
{
	if (running_)
	{	
		return;
	}

	joinWorkerThread();

	running_ = true;

	std::lock_guard<std::mutex> lock(mutex_);

    workerThread_ = std::thread(&ATimer::run, this);
}

// Stop the timer thread
void ATimer::stop() noexcept
{
	std::lock_guard<std::mutex> lock(mutex_);

	if (!running_)
	{
		return;
	}

	running_ = false;
	control_variable_.notify_one();
}

void ATimer::scheduleOnce(std::chrono::milliseconds duration, Callback callback)
{
	std::lock_guard<std::mutex> lock(mutex_);

	delay_ = duration;

	if (callback)
	{
		callback_ = std::move(callback);
	}

	one_shot_ = true;

	control_variable_.notify_one();
}

void ATimer::scheduleRepeating(std::chrono::milliseconds duration, Callback callback)
{
	std::lock_guard<std::mutex> lock(mutex_);

	delay_ = duration;

	if (callback)
	{
		callback_ = std::move(callback);
	}

	one_shot_ = false;

	control_variable_.notify_one();
}

void ATimer::setCallback(Callback callback)
{
	// Callback can be used in the worker thread
	std::lock_guard<std::mutex> lock(mutex_);
	if (callback)
	{
		callback_ = std::move(callback);
	}
}

void ATimer::joinWorkerThread()
{
	if (workerThread_.joinable())
	{
		workerThread_.join();
	}
}

void ATimer::forceStop()
{
    stop();
    joinWorkerThread();
}

ATimer::~ATimer()
{
	forceStop();
	// RAII for the worker thread - it may not be joined from the forceStop() if not running !
	joinWorkerThread();
}

}// namespace Core