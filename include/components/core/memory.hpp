#pragma once

#include <array>
#include <vector>

#include "utils/bitwise_utils.hpp"

class memory
{
public:
    memory(uint16_t program_start_byte = default_program_start_byte)
        : program_start_byte(program_start_byte) {};

    uint8_t& operator[](uint16_t index)
    {
        return mem[index];
    }

    uint16_t get_dword(uint16_t index)
    {
        return from_bytes(mem[index], mem[index + 1]);
    }

    void set_dword(uint16_t value, uint16_t index)
    {
        auto [first, second] = to_bytes(value);

        mem[index]     = first;
        mem[index + 1] = second;
    }

    std::vector<uint8_t> bytes(uint16_t start, uint16_t count)
    {
        auto start_it = mem.begin() + start;

        return std::vector<uint8_t>(start_it, start_it + count);
    }

    uint16_t program_start_byte;

    static constexpr uint16_t size = 4096;
    static constexpr uint16_t default_program_start_byte = 512;

    std::array<uint8_t, size> mem = {0xF0, 0x90, 0x90, 0x90, 0xF0,
                                     0x20, 0x60, 0x20, 0x20, 0x70,
                                     0xF0, 0x10, 0xF0, 0x80, 0xF0,
                                     0xF0, 0x10, 0xF0, 0x10, 0xF0,
                                     0x90, 0x90, 0xF0, 0x10, 0x10,
                                     0xF0, 0x80, 0xF0, 0x10, 0xF0,
                                     0xF0, 0x80, 0xF0, 0x90, 0xF0,
                                     0xF0, 0x10, 0x20, 0x40, 0x40,
                                     0xF0, 0x90, 0xF0, 0x90, 0xF0,
                                     0xF0, 0x90, 0xF0, 0x10, 0xF0,
                                     0xF0, 0x90, 0xF0, 0x90, 0x90,
                                     0xE0, 0x90, 0xE0, 0x90, 0xE0,
                                     0xF0, 0x80, 0x80, 0x80, 0xF0,
                                     0xE0, 0x90, 0x90, 0x90, 0xE0,
                                     0xF0, 0x80, 0xF0, 0x80, 0xF0,
                                     0xF0, 0x80, 0xF0, 0x80, 0x80};
};
