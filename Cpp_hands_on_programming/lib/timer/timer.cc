#include "timer.h"
#include <vector>
#include <algorithm>
#include <thread>

void Timer::startTimer() {
	start = std::chrono::system_clock::now();
}

void Timer::checkTimer(std::string error, std::string msg) {
	//check timer
	end = std::chrono::system_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
	if (duration <= timeLimit) {
		if (duration != 0 && duration % 1000 == 0) {
			std::cout << msg << std::endl;
			std::this_thread::sleep_for (std::chrono::milliseconds(1)); // make sure one millisecond has passed for next iteration
		}
	} else {
		throw(std::runtime_error("Time out! " + error));
	}
}

const timespec& Timer::useTimespec() {
	clock_gettime(CLOCK_REALTIME, &time);
	time.tv_sec += 10;
	return time;
}