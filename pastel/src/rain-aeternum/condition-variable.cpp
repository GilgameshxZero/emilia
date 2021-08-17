#include "condition-variable.h"

namespace Rain {
	ConditionVariable::ConditionVariable() {
		std::condition_variable::condition_variable();

		wakesLeft = 0;
	}

	//overrides
	void ConditionVariable::wait(std::unique_lock<std::mutex> &lck) {
		std::condition_variable::wait(lck, std::bind(&ConditionVariable::predicate, this));
	}

	void ConditionVariable::notify_one() noexcept {
		wakesLeft++;
		std::condition_variable::notify_one();
	}

	void ConditionVariable::notify_all() noexcept {
		wakesLeft = -1;
		std::condition_variable::notify_all();
	}

	void ConditionVariable::unNotifyAll() {
		wakesLeft = 0;
	}

	std::mutex &ConditionVariable::getMutex() {
		return m;
	}

	bool ConditionVariable::predicate() {
		if (wakesLeft == -1) {
			return true;
		} else if (wakesLeft > 0) {
			wakesLeft--;
			return true;
		} else {
			return false;
		}
	}
}