#include "../include/TimerManager.h"
#include <iostream>
#include <chrono>

TimerManager::TimerManager() {
    timerThread = std::thread(&TimerManager::timerThreadFunction, this);
}

TimerManager::~TimerManager() {
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        stopThread = true;
        timerCondition.notify_all();
    }
    if (timerThread.joinable()) {
        timerThread.join();
    }
}

void TimerManager::setTimer(int duration, const std::string& action) {
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        timerQueue.push({duration, action});
    }
    timerCondition.notify_all();
    std::cout << "Timer set for " << duration << " seconds to execute: " << action << "\n";
}

void TimerManager::cancelAllTimers() {
    {
        std::lock_guard<std::mutex> lock(timerMutex);
        cancelCurrentTimer = true;
        while (!timerQueue.empty()) {
            timerQueue.pop();
        }
    }
    timerCondition.notify_all();
    std::cout << "All timers canceled.\n";
}

void TimerManager::registerCallback(const std::function<void(const std::string&)>& callback) {
    actionCallback = callback;
}

void TimerManager::timerThreadFunction() {
    while (true) {
        TimerRequest request;
        {
            std::unique_lock<std::mutex> lock(timerMutex);
            timerCondition.wait(lock, [this] { return !timerQueue.empty() || stopThread; });
            if (stopThread && timerQueue.empty()) break;
            request = timerQueue.front();
            timerQueue.pop();
        }

        std::this_thread::sleep_for(std::chrono::seconds(request.duration));
        if (!cancelCurrentTimer && actionCallback) {
            actionCallback(request.action);
        }
    }
}
