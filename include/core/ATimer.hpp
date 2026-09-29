/**
 * @file ATimer.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 *
 */

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>


/**
 * @brief 
 * 
 * @page ATimerPage ATimer Class
 * 
 * @brief Lightweight asynchronous timer executing a callback after a delay.
 * 
 * @section timer Overview
 * 
 * ATimer provides a thread-based timer capable of executing a user callback
 * either once (single-shot) or periodically. The timer internally runs a
 * worker thread which waits on a condition variable until the next scheduled
 * firing time.
 * 
 * The callback is always executed outside the internal mutex to avoid
 * blocking timer control operations or causing deadlocks.
 * The timer uses std::chrono::steady_clock to avoid time adjustments caused
 * by system clock changes.
 * 
 * @section Thread-safety
 * 
 * - start() and stop() may be called from different threads.
 * - The callback may execute concurrently with control functions.
 * - Callback replacement via setCallback() is synchronized.
 * 
 * @section Lifecycle
 * 
 * - The worker thread is created when start() is called.
 * - stop() stops the timer and joins the worker thread.
 * - The destructor ensures the worker thread is stopped and joined.
 * 
 * @section Behavior
 * 
 * - In single-shot mode the callback executes once and the timer stops.
 * - In repeating mode the callback executes periodically.
 * - Delay changes wake the worker thread and reschedule the next execution.
 * 
 * @section timing Timing model
 * 
 * - The timer uses an absolute next-fire deadline to prevent drift between
 *   iterations.
 * 
 * @section Limitations
 * 
 * - Callback execution time is not compensated; long callbacks delay the
 *   scheduling of the next iteration.
 * - The timer resolution depends on the underlying OS scheduler.
 * 
 * @section References
 * 
 * - Core::ATimer
 */

namespace Core
{

class ATimer final
{
	using Clock = std::chrono::steady_clock;
	using Duration = Clock::duration;
	using Callback = std::function<void()>;

public:
	/**
	 * @brief Construct a new ATimer object
	 * 
	 * @param duration 
	 * @param callback std::function<void()>
	 * @param singleShot 
	 */
	ATimer(std::chrono::milliseconds duration, Callback callback, bool singleShot = true)
		:
		one_shot_{ singleShot },
		delay_{ duration },
		callback_{ std::move(callback) }
	{
	}

	~ATimer();

	// Delete copy operations
	ATimer(const ATimer&) = delete;
	ATimer& operator=(const ATimer&) = delete;

	/**
	 * @brief Schedule a one-shot timer
	 * 
	 * @param duration 
	 * @param callback 
	 */
	void scheduleOnce(std::chrono::milliseconds duration, Callback callback=nullptr);

	/**
	 * @brief Schedule a repeating timer
	 * 
	 * @param duration 
	 * @param callback std::function<void()>
	 */
	void scheduleRepeating(std::chrono::milliseconds duration, Callback callback=nullptr);

	/**
	 * @brief Start the timer thread
	 * 
	 */
	void start();

	/**
	 * @brief Stop the timer thread 
	 * @note Can be used from the timer callback
	 */
	void stop() noexcept;

	/**
	 * @brief Set the Callback object
	 * 
	 * @param callback 
	 */
	void setCallback(Callback callback);

	/**
	 * @brief Set the periodic duration
	 * 
	 * @param duration 
	 */
	void setDelay(std::chrono::milliseconds duration)
	{
		// Delay can be set from another thread, respects TOCTOU = Time-of-Check Time-of-Use
		std::lock_guard<std::mutex> lock(mutex_);
		delay_ = duration;
	}

	/**
	 * @brief Set timer periodic behavior - singleShot=true disables periodic behavior
	 * 
	 * @param singleShot 
	 */
	void setSingleShot(bool singleShot)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		one_shot_ = singleShot;
	}

private:
	void forceStop(); 
	void joinWorkerThread();
	/**
	 * @brief Internal run method
	 * 
	 */
	void run();

	std::atomic<bool> running_{ false };
	std::atomic<bool> one_shot_;
	Duration delay_;
	std::thread workerThread_;
	std::mutex mutex_;
	std::condition_variable control_variable_;
	Callback callback_;
};

}// namespace Core

