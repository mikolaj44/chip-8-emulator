#pragma once

#include <iostream>
#include <format>

class keyboard
{
public:
    bool key_pressed(uint8_t keycode)
    {
        std::cout << std::format("[KEYBOARD] key_pressed called with keycode {}", keycode) << "\n";

        return false;
    }

    uint8_t wait_for_key_pressed()
    {
        std::cout << "[KEYBOARD] wait_for_key_pressed called" << "\n";

        return 0;
    }
};
