#pragma once

#include <iostream>

class buzzer
{
public:
    void start()
    {
        std::cout << "[BUZZER] starting buzzer" << "\n";
    }

    void stop()
    {
        std::cout << "[BUZZER] stopping buzzer" << "\n";
    }
};
