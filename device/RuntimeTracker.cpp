#include "RuntimeTracker.h"
#include <chrono>
#include <iostream>
#include <ctime>

using namespace std;

// Constructor
RuntimeTracker::RuntimeTracker() {
    lastUpdate = std::time(nullptr); // Initialize the last update time to now
}

// Destructor
RuntimeTracker::~RuntimeTracker() {}

// Reset daily, monthly, and yearly runtimes if a new period has started
void RuntimeTracker::resetIfNewPeriod() {
    std::time_t now = std::time(nullptr);
    std::tm *currentTime = std::localtime(&now);
    std::tm *lastTime = std::localtime(&lastUpdate);

    // Reset daily runtime if the day has changed
    if (currentTime->tm_yday != lastTime->tm_yday) {
        dailyRuntime = 0;
    }

    // Reset monthly runtime if the month has changed
    if (currentTime->tm_mon != lastTime->tm_mon) {
        monthlyRuntime = 0;
    }

    // Reset yearly runtime if the year has changed
    if (currentTime->tm_year != lastTime->tm_year) {
        yearlyRuntime = 0;
    }

    lastUpdate = now; // Update the last update time
}

// Start the timer for runtime tracking
void RuntimeTracker::startTimer(int power) {
    lastStartTime = chrono::system_clock::now();
    currentPower = power;
    cout << "Timer started with power: " << power << "W\n"; // Debug output
}

// Stop the timer and update runtime and power consumption
void RuntimeTracker::stopTimer() {
    auto now = chrono::system_clock::now();
    auto duration = chrono::duration_cast<chrono::seconds>(now - lastStartTime).count();

    resetIfNewPeriod(); // Check if a new period has started

    totalRuntime += duration;
    dailyRuntime += duration;
    monthlyRuntime += duration;
    yearlyRuntime += duration;
    cumulativePowerConsumption += duration * currentPower;

    currentPower = 0; // Reset power
    cout << "Timer stopped. Duration: " << duration << " seconds\n"; // Debug output
}

// Update runtime dynamically
void RuntimeTracker::updateRuntime() {
    resetIfNewPeriod(); // Ensure runtime is accurate for the current period
}

// Accessors
int RuntimeTracker::getDailyRuntime() const {
    return dailyRuntime;
}

int RuntimeTracker::getMonthlyRuntime() const {
    return monthlyRuntime;
}

int RuntimeTracker::getYearlyRuntime() const {
    return yearlyRuntime;
}

int RuntimeTracker::getCumulativePowerConsumption() const {
    return cumulativePowerConsumption;
}
