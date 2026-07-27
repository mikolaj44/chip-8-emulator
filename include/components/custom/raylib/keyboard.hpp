#pragma once

#include <cstdint>
#include <unordered_map>

#include <raylib.h>

// this class depends on the raylib window being already initialized
// not pretty but avoids a headache
class keyboard
{
public:
    keyboard() = default;

    bool key_pressed(uint8_t keycode)
    {
        return IsKeyDown(ascii_key_map[keycode]);
    }

    uint8_t wait_for_key_pressed()
    {
        static constexpr uint8_t invalid_key = 16;

        int pressed_key = invalid_key;
        uint8_t mapped_pressed_key = invalid_key;

        while(true)
        {
            if(pressed_key != invalid_key && IsKeyReleased(pressed_key))
            {
                return mapped_pressed_key;
            }

            for (const auto& key_pair : ascii_key_map)
            {
                if(IsKeyDown(key_pair.second))
                {
                    mapped_pressed_key = key_pair.first;
                    pressed_key = key_pair.second;

                    break;
                }
            }
        }
    }

private:
    std::unordered_map<uint8_t, int> ascii_key_map =
    {{{0, KEY_ZERO},  {1, KEY_ONE}, {2, KEY_TWO}, {3, KEY_THREE}, {4, KEY_FOUR},
      {5, KEY_FIVE}, {6, KEY_SIX}, {7, KEY_SEVEN}, {8, KEY_EIGHT}, {9, KEY_NINE},
      {0xA, KEY_A}, {0xB, KEY_B}, {0xC, KEY_C}, {0xD, KEY_D}, {0xE, KEY_E}, {0xF, KEY_F}}};
};
