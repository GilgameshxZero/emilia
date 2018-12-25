/*
Standard
*/

#pragma once

#include <condition_variable>

namespace Rain {
	//wrapper class around std::condition variable safe against spurious wakeups
	class ConditionVariable : public std::condition_variable {
	public:
		ConditionVariable();

		//overrides
		void wait(std::unique_lock<std::mutex> &lck);

		template <class Rep, class Period>
		std::cv_status wait_for(std::unique_lock<std::mutex>& lck,
			const std::chrono::duration<Rep, Period>& rel_time) {
			std::condition_variable::wait_for(lck, rel_time, std::bind(&ConditionVariable::predicate, this));
		}

		template <class Clock, class Duration>
		std::cv_status wait_until(std::unique_lock<std::mutex>& lck,
			const std::chrono::time_point<Clock, Duration>& abs_time) {
			std::condition_variable::wait_for(lck, abs_time, std::bind(&ConditionVariable::predicate, this));
		}

		//makes no sense to call this after notify_all
		void notify_one() noexcept;

		void notify_all() noexcept;

		//no longer wakes threads
		void unNotifyAll();

		//get mutex for unique lock
		std::mutex &getMutex();

	private:
		int wakesLeft; //-1 means to wake infinite threads
		std::mutex m;

		bool predicate();
	};
}