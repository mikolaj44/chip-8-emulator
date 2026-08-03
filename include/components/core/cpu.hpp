#pragma once

#include <cstdint>
#include <random>
#include <thread>
#include <vector>

#include "components/core/memory.hpp"
#include "components/core/timer.hpp"
#include "utils/time_utils.hpp"
#include "utils/function_call_proxy.hpp"

template<typename Screen, typename Keyboard, typename Buzzer>
class cpu
{
    using reg8 = uint8_t;
    using reg16 = uint16_t;

public:
    cpu(memory, Screen, Keyboard, Buzzer,
        size_t cpu_frequency_hz = default_cpu_frequency_hz,
        size_t timers_frequency_hz = default_timers_frequency_hz,
        size_t screen_frequency_hz = default_screen_frequency_hz);

    ~cpu();
    cpu(const cpu&);

    void start();
    void stop();

    // general purpose registers
    std::array<reg8, 16> reg{};
    uint8_t& reg_F = reg[15];

    // memory address
    reg16 reg_I{};

    // program counter, stack pointer
    reg16 PC;
    reg8 SP{};

    // timers
    timer sound_timer;
    timer delay_timer;

    static constexpr size_t default_cpu_frequency_hz = 500;
    static constexpr size_t default_timers_frequency_hz = 60;
    static constexpr size_t default_screen_frequency_hz = 60;

private:
    memory mem;
    Screen screen;
    Keyboard keyboard;
    Buzzer buzzer;

    bool is_initialized = false;
    bool is_stopped = true;
    bool buzzer_is_playing = false;

    size_t cpu_frequency_hz;
    size_t timers_frequency_hz;
    size_t screen_frequency_hz;

    size_t cpu_sleep_ms;
    size_t timer_sleep_ms;
    size_t screen_sleep_ms;

    timestamp prev_cpu_execute_ts = current_timestamp();
    timestamp prev_timer_decrement_ts = current_timestamp();
    timestamp prev_screen_refresh_ts = current_timestamp();

    std::random_device dev;
    std::mt19937 rng{dev()};
    std::uniform_int_distribution<std::mt19937::result_type> dist_255{0, 255};

    std::thread main_execution_thread;
    std::thread screen_thread;
    std::thread keyboard_thread;
    std::thread buzzer_thread;

    void main_execution_loop();
    void screen_loop();
    void keyboard_loop();
    void buzzer_loop();

    function_call_proxy<bool> screen_clear_proxy{
        {[&] () {screen.clear(); return true;}}
    };

    function_call_proxy<bool, uint8_t, uint8_t, std::vector<uint8_t>> screen_draw_proxy{
        {[&] (uint8_t x, uint8_t y, std::vector<uint8_t> bytes) {return screen.draw(x, y, bytes);}}
    };

    function_call_proxy<bool, uint8_t> keyboard_key_pressed_proxy{
        {[&] (uint8_t keycode) {return keyboard.key_pressed(keycode);}}
    };

    function_call_proxy<uint8_t> keyboard_wait_for_key_pressed_proxy{
        {[&] () {return keyboard.wait_for_key_pressed();}}
    };

    using operation = void (cpu::*)(uint16_t);
    operation get_operation(uint16_t opcode);

    void call_machine(uint16_t);
    void clear(uint16_t);
    void ret(uint16_t);
    void jump(uint16_t);
    void call_sub(uint16_t);
    void skip_eq_imm(uint16_t);
    void skip_neq_imm(uint16_t);
    void skip_eq_reg(uint16_t);
    void load_imm(uint16_t);
    void add_imm(uint16_t);
    void load_reg(uint16_t);
    void or_reg(uint16_t);
    void and_reg(uint16_t);
    void xor_reg(uint16_t);
    void add_reg(uint16_t);
    void sub_reg(uint16_t);
    void shr(uint16_t);
    void subn(uint16_t);
    void shl(uint16_t);
    void skip_neq_reg(uint16_t);
    void load_addr(uint16_t);
    void jump_addr(uint16_t);
    void rnd(uint16_t);
    void draw(uint16_t);
    void skip_eq_key(uint16_t);
    void skip_neq_key(uint16_t);
    void load_delay(uint16_t);
    void load_key(uint16_t);
    void set_delay(uint16_t);
    void set_sound(uint16_t);
    void add_i(uint16_t);
    void load_sprite(uint16_t);
    void load_bcd(uint16_t);
    void copy_reg(uint16_t);
    void read_reg(uint16_t);
};

#include "cpu.tpp"
