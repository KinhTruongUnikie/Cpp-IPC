#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <iostream>

class Timer {
private:
    int timeLimit;
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;
    timespec time; // struct timespec
public:
    Timer(int ms = 10000) : timeLimit(ms){} //milliseconds
    void startTimer();
    void checkTimer(std::string error, std::string msg = "");
    const timespec& useTimespec();
};

#endif