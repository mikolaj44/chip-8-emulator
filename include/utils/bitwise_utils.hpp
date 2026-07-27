#pragma once

#include <cstdint>
#include <type_traits>
#include <tuple>

namespace
{
template<typename T>
concept uint_16_or_8 = std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>;

template<typename T> requires uint_16_or_8<T>
constexpr T lsb_mask = (std::is_same_v<T, uint8_t> ? 0x0001 : 0x00000001);

template<typename T> requires uint_16_or_8<T>
constexpr T msb_mask = (std::is_same_v<T, uint8_t> ? 0x80 : 0x8000);

template<typename T> requires uint_16_or_8<T>
constexpr T msb_shift = (std::is_same_v<T, uint8_t> ? 7 : 15);
}

uint16_t lowest_12_bits(uint16_t value)
{
    return value & 0x0FFF;
}

uint8_t lowest_byte(uint16_t value)
{
    return value & 0x00FF;
}

template<typename T> requires uint_16_or_8<T>
uint8_t first_nibble(T value)
{
    return (value & 0xF000) >> 12;
}

template<typename T> requires uint_16_or_8<T>
uint8_t second_nibble(T value)
{
    return (value & 0x0F00) >> 8;
}

uint8_t third_nibble(uint16_t value)
{
    return (value & 0x00F0) >> 4;
}

uint8_t fourth_nibble(uint16_t value)
{
    return value & 0x000F;
}

std::pair<uint8_t, uint8_t> middle_nibbles(uint16_t value)
{
    return {second_nibble(value), third_nibble(value)};
}

std::pair<uint8_t, uint8_t> second_nibble_byte(uint16_t value)
{
    return {second_nibble(value), lowest_byte(value)};
}

std::tuple<uint8_t, uint8_t, uint8_t> three_nibbles_from_second(uint16_t value)
{
    return {second_nibble(value), third_nibble(value), fourth_nibble(value)};
}

template<typename T> requires uint_16_or_8<T>
uint8_t lsb(T value)
{
    return value & lsb_mask<T>;
}

template<typename T> requires uint_16_or_8<T>
uint8_t msb(T value)
{
    return (value & msb_mask<T>) >> msb_shift<T>;
}

uint8_t bit_at(uint8_t value, uint8_t pos)
{
    return (value >> pos) & 1;
}

uint16_t from_bytes(uint8_t high, uint8_t low)
{
    return (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);
}

std::pair<uint8_t, uint8_t> to_bytes(uint16_t value)
{
    return {static_cast<uint8_t>((value & 0xFF00) >> 8), static_cast<uint8_t>(value & 0x00FF)};
}
