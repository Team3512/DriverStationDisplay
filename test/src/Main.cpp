// Copyright (c) 2017 FRC Team 3512, Spartatroniks. All Rights Reserved.

#include <chrono>
#include <iostream>
#include <thread>

#include "DSDisplay/DSDisplay.hpp"

using namespace std::chrono_literals;

class Robot {
public:
    void func1() { std::cout << "Auto 1" << std::endl; }

    void func2() { std::cout << "Auto 2" << std::endl; }

    void func3() { std::cout << "Auto 3" << std::endl; }
};

int main() {
    DSDisplay& dsDisplay{DSDisplay::GetInstance(5800)};
    Robot robot;

    dsDisplay.AddAutoMethod("Auto 1", std::bind(&Robot::func1, &robot));
    dsDisplay.AddAutoMethod("Auto 2", std::bind(&Robot::func2, &robot));
    dsDisplay.AddAutoMethod("Auto 3", std::bind(&Robot::func3, &robot));

    auto lastTime = std::chrono::steady_clock::now();
    auto currentTime = lastTime;

    while (1) {
        dsDisplay.ReceiveFromDS();

        currentTime = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime -
                                                                  lastTime)
                .count() > 500) {
            // Send things to DS display
            dsDisplay.Clear();

            dsDisplay.AddData("ENCODER_LEFT", -3.5);
            dsDisplay.AddData("ENCODER_RIGHT", -3.1);
            dsDisplay.AddData("EV_POS_DISP", 42);
            dsDisplay.AddData("EV_POS", 100 * 42 / 60);
            dsDisplay.AddData("GYRO_VAL", 30.4);

            dsDisplay.AddData("ARMS_CLOSED", true);
            dsDisplay.AddData("INTAKE_ARMS_CLOSED", false);
            dsDisplay.AddData("CONTAINER_GRABBER_CLOSED", true);

            dsDisplay.SendToDS();

            lastTime = currentTime;
        }

        std::this_thread::sleep_for(100ms);
    }
}
