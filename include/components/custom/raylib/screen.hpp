#pragma once

#include <vector>
#include <climits>

#include "utils/bitwise_utils.hpp"

#include <raylib.h>

class screen
{
public:
    screen(size_t pixel_size = default_pixel_size, Color on_color = default_on_color, Color off_color = default_off_color)
        : pixel_size(pixel_size),
          width_pixels(SCREEN_WIDTH * pixel_size),
          height_pixels(SCREEN_HEIGHT * pixel_size),
          on_color(on_color),
          off_color(off_color)
        {};

    ~screen()
    {
        if(!texture_initialized)
        {
            return;
        }

        UnloadRenderTexture(screen_texture);
        UnloadRenderTexture(prev_screen_texture);

        CloseWindow();
    }

    void setup()
    {
        InitWindow(width_pixels, height_pixels, "CHIP-8");

        SetTraceLogLevel(LOG_ERROR);

        screen_texture = LoadRenderTexture(width_pixels, height_pixels);
        prev_screen_texture = LoadRenderTexture(width_pixels, height_pixels);

        texture_initialized = true;

        clear();
    }

    void start()
    {
        MaximizeWindow();
    }

    void stop()
    {
        MinimizeWindow();
    }

    void clear()
    {
        BeginTextureMode(screen_texture);

        ClearBackground(off_color);

        EndTextureMode();

        assign_texture(prev_screen_texture, screen_texture);
    }

    // returns true if any bits get flipped from 1 to 0 when drawing, otherwise false
    bool draw(uint8_t x, uint8_t y, std::vector<uint8_t> bytes)
    {
        Image prev_screen = LoadImageFromTexture(prev_screen_texture.texture);

        bool pixels_flipped = false;

        for(size_t byte_index = 0; byte_index < bytes.size(); byte_index++)
        {
            for(uint8_t bit_index = 0; bit_index < CHAR_BIT; bit_index++)
            {
                size_t col = x + bit_index;
                size_t row = y + byte_index;

                size_t pixel_x = col * pixel_size;
                size_t pixel_y = row * pixel_size;

                const Color prev_color = GetImageColor(prev_screen, pixel_x, pixel_y);

                bool bit_set = static_cast<bool>(bit_at(bytes[byte_index], 7 - bit_index));
                bool prev_bit_set = (ColorToInt(prev_color) == ColorToInt(on_color));

                Color color = off_color;

                // pixel drawing: prev XOR curr
                if(bit_set)
                {
                    if(prev_bit_set)
                    {
                        pixels_flipped = true;
                    }
                    else
                    {
                        color = on_color;
                    }
                }
                else if(prev_bit_set)
                {
                    color = on_color;
                }

                BeginTextureMode(screen_texture);

                DrawRectangle(pixel_x, pixel_y, pixel_size, pixel_size, color);

                EndTextureMode();
            }
        }

        assign_texture(prev_screen_texture, screen_texture);

        UnloadImage(prev_screen);

        return pixels_flipped;
    }

    void update()
    {
        BeginDrawing();

        draw_texture(screen_texture, true);

        EndDrawing();
    }

    static constexpr size_t default_pixel_size = 16;
    static constexpr Color default_on_color = GREEN;
    static constexpr Color default_off_color = DARKPURPLE;

private:
    const size_t pixel_size;
    const size_t width_pixels;
    const size_t height_pixels;

    const Color on_color  = GREEN;
    const Color off_color = DARKPURPLE;

    RenderTexture2D screen_texture;
    RenderTexture2D prev_screen_texture;

    bool texture_initialized = false;

    constexpr static size_t SCREEN_WIDTH  = 64;
    constexpr static size_t SCREEN_HEIGHT = 32;

    void assign_texture(RenderTexture2D& left, RenderTexture2D& right)
    {
        BeginTextureMode(left);

        draw_texture(right);

        EndTextureMode();
    }

    void draw_texture(const RenderTexture2D& texture, bool flip = false)
    {
        int mult = (flip ? -1 : 1);

        DrawTextureRec(texture.texture, { 0, 0, static_cast<float>(width_pixels), mult * static_cast<float>(height_pixels) }, { 0, 0 }, WHITE);
    }
};
