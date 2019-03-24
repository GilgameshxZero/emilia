/*
Standard
*/

#pragma once

#include "condition-variable.h"
#include "utility-time.h"

namespace Rain {
	class Timer {
	public:
		Timer();

		//manage time left in timer
		void addTime(unsigned int ms);
		unsigned int getTime();

		//setTime can only increase time, not decrease it
		void setTime(unsigned int ms);

		//wait until timer expires
		void wait();

	private:
		ConditionVariable cv;
		unsigned int timeLeft;
	};
}