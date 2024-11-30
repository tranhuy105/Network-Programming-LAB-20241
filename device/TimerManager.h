#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>

struct TimerRequest {
    int duration; // Seconds
    std::string action;
};

class TimerManager {
private:
    std::queue<TimerRequest> timerQueue;
    std::thread timerThread;
    std::mutex timerMutex;
    std::condition_variable timerCondition;
    bool stopThread = false;
    bool cancelCurrentTimer = false;

    // Callback to execute actions
    std::function<void(const std::string&)> actionCallback;

    void timerThreadFunction();

public:
    TimerManager();
    ~TimerManager();

    void setTimer(int duration, const std::string& action);
    void cancelAllTimers();

    void registerCallback(const std::function<void(const std::string&)>& callback);
};

#endif // TIMER_MANAGER_H
