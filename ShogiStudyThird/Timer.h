#pragma once
#include <chrono>

class Timer {
public:
	Timer() {

	}
	void start() {
		startTime = std::chrono::system_clock::now();
	}
	void end() {
		endTime = std::chrono::system_clock::now();
	}
	//ミリ秒
	double get() {
		double r = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0);
		return r;
	}
private:
	std::chrono::system_clock::time_point startTime, endTime;
};
