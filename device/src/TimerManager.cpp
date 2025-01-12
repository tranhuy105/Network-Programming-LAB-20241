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
        // join để đảm bảo thread đã kết thúc trước khi hủy đối tượng TimerManager
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
            // Tạo phạm vi riêng để quản lý mutex
            std::unique_lock<std::mutex> lock(timerMutex);

            // Đợi cho đến khi:
            // - Có tác vụ trong hàng đợi `timerQueue`, hoặc
            // - Có tín hiệu dừng `stopThread` được đặt thành true
            timerCondition.wait(lock, [this] { 
                return !timerQueue.empty() || stopThread; 
            });

            // Nếu tín hiệu dừng (`stopThread`) được kích hoạt và không còn tác vụ nào, thoát khỏi vòng lặp
            if (stopThread && timerQueue.empty()) 
                break;

            // Lấy tác vụ từ hàng đợi `timerQueue`
            request = timerQueue.front();
            timerQueue.pop();
        } // Mở khóa mutex ở đây để các thread khác có thể truy cập `timerQueue`

        // Tiến hành đợi cho đến khi tác vụ được hoàn thành (thời gian `duration` giây)
        std::this_thread::sleep_for(std::chrono::seconds(request.duration));

        // Sau khi hết thời gian, nếu không có lệnh hủy timer (`cancelCurrentTimer`)
        // và callback được đăng ký (`actionCallback`), thực hiện hành động được yêu cầu
        if (!cancelCurrentTimer && actionCallback) {
            actionCallback(request.action);
        }
    }
}
