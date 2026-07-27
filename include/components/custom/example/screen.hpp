#pragma once

#include <vector>
#include <iostream>

class screen
{
public:
    void clear()
    {
        std::cout << "[SCREEN] clearing screen" << "\n";
    }

    bool draw(uint8_t x, uint8_t y, std::vector<uint8_t> bytes)
    {
        std::cout << std::format("[SCREEN] draw called with x: {}, y: {} for {} bytes", x, y, bytes.size()) << "\n";

        return false;
    }

    void refresh()
    {
        std::cout << "[SCREEN] refreshing screen" << "\n";
    }
};
