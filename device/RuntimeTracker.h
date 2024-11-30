#ifndef RUNTIMETRACKER_H
#define RUNTIMETRACKER_H

#include <chrono>
#include <ctime>

class RuntimeTracker {
private:
    int totalRuntime = 0;                  // Total runtime in seconds
    int dailyRuntime = 0;                  // Daily runtime in seconds
    int monthlyRuntime = 0;                // Monthly runtime in seconds
    int yearlyRuntime = 0;                 // Yearly runtime in seconds
    int cumulativePowerConsumption = 0;    // Total power consumption in Watts
    int currentPower = 0;                  // Current power consumption in Watts

    std::chrono::time_point<std::chrono::system_clock> lastStartTime; // Timer start
    std::time_t lastUpdate;                // Last time the runtime was updated

    void resetIfNewPeriod();

public:
    RuntimeTracker();
    ~RuntimeTracker();

    // Power and runtime management
    void startTimer(int power);
    void stopTimer();
    void updateRuntime();

    int getDailyRuntime() const;
    int getMonthlyRuntime() const;
    int getYearlyRuntime() const;
    int getCumulativePowerConsumption() const;
};

#endif // RUNTIMETRACKER_H
