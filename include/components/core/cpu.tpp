#pragma once

#include <cstdint>
#include <unordered_map>

#include "cpu.hpp" // for code formatting

template<typename Screen, typename Keyboard, typename Buzzer>
cpu<Screen, Keyboard, Buzzer>::cpu(memory mem, Screen screen, Keyboard keyboard, Buzzer buzzer,
                                   size_t cpu_frequency_hz, size_t timers_frequency_hz, size_t screen_frequency_hz)
    : PC(mem.program_start_byte),
      mem(mem),
      screen(screen),
      keyboard(keyboard),
      buzzer(buzzer),
      cpu_frequency_hz(cpu_frequency_hz),
      timers_frequency_hz(timers_frequency_hz),
      screen_frequency_hz(screen_frequency_hz),
      cpu_sleep_ms(1000 / cpu_frequency_hz),
      timer_sleep_ms(1000 / timers_frequency_hz),
      screen_sleep_ms(1000 / screen_frequency_hz)
      {}

template<typename Screen, typename Keyboard, typename Buzzer>
cpu<Screen, Keyboard, Buzzer>::cpu(const cpu<Screen, Keyboard, Buzzer>& other)
    : cpu(other.mem, other.screen, other.keyboard, other.buzzer,
          other.cpu_frequency_hz, other.timers_frequency_hz, other.screen_frequency_hz) {}

template<typename Screen, typename Keyboard, typename Buzzer>
cpu<Screen, Keyboard, Buzzer>::~cpu()
{
    stop();
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::start()
{
    if(!is_stopped)
    {
        return;
    }

    is_stopped = false;

    main_execution_thread = std::thread([this] {this->main_execution_loop();});
    screen_thread = std::thread([this] {this->screen_loop();});
    keyboard_thread = std::thread([this] {this->keyboard_loop();});
    buzzer_thread = std::thread([this] {this->buzzer_loop();});
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::stop()
{
    if(is_stopped)
    {
        return;
    }

    is_stopped = true;

    if constexpr (requires {screen.stop();} )
    {
        screen.stop();
    }
    if constexpr (requires {keyboard.stop();} )
    {
        keyboard.stop();
    }

    buzzer.stop();

    main_execution_thread.join();
    screen_thread.join();
    keyboard_thread.join();
    buzzer_thread.join();

    if(!is_initialized)
    {
        is_initialized = true;
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::main_execution_loop()
{
    while(!is_stopped)
    {
        uint16_t opcode = mem.get_dword(PC);

        (this->*get_operation(opcode))(opcode);

        PC += 2;

        std::this_thread::sleep_for(std::chrono::milliseconds(cpu_sleep_ms));
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::screen_loop()
{
    if(!is_initialized)
    {
        if constexpr (requires {screen.setup();} )
        {
            screen.setup();
        }
    }

    if constexpr (requires {screen.start();} )
    {
        screen.start();
    }

    while(!is_stopped)
    {
        if(screen_clear_proxy.was_set())
        {
            screen_clear_proxy.call();
        }

        if(screen_draw_proxy.was_set())
        {
            screen_draw_proxy.call();
        }

        screen.update();

        std::this_thread::sleep_for(std::chrono::milliseconds(screen_sleep_ms));
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::keyboard_loop()
{
    if(!is_initialized)
    {
        if constexpr (requires {keyboard.setup();} )
        {
            keyboard.setup();
        }
    }

    if constexpr (requires {keyboard.start();} )
    {
        keyboard.start();
    }

    while(!is_stopped)
    {
        if(keyboard_key_pressed_proxy.was_set())
        {
            keyboard_key_pressed_proxy.call();
        }

        if(keyboard_wait_for_key_pressed_proxy.was_set())
        {
            keyboard_wait_for_key_pressed_proxy.call();
        }

        if constexpr (requires {keyboard.update();} )
        {
            keyboard.update();
        }
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::buzzer_loop()
{
    if(!is_initialized)
    {
        if constexpr (requires {buzzer.setup();} )
        {
            buzzer.setup();
        }
    }

    while(!is_stopped)
    {
        if(static_cast<uint8_t>(sound_timer) != 0 && !buzzer_is_playing)
        {
            buzzer.start();

            buzzer_is_playing = true;
        }
        else if(static_cast<uint8_t>(sound_timer) == 0 && buzzer_is_playing)
        {
            buzzer.stop();

            buzzer_is_playing = false;
        }

        sound_timer.decrement();
        delay_timer.decrement();

        if constexpr (requires {buzzer.update();} )
        {
            buzzer.update();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(timer_sleep_ms));
    }
}

// normally would be all done with an opcode table
template<typename Screen, typename Keyboard, typename Buzzer>
cpu<Screen, Keyboard, Buzzer>::operation cpu<Screen, Keyboard, Buzzer>::get_operation(uint16_t opcode)
{
    static std::vector<operation> first_1_to_7 =
        {   &cpu<Screen, Keyboard, Buzzer>::jump,
            &cpu<Screen, Keyboard, Buzzer>::call_sub,
            &cpu<Screen, Keyboard, Buzzer>::skip_eq_imm,
            &cpu<Screen, Keyboard, Buzzer>::skip_neq_imm,
            &cpu<Screen, Keyboard, Buzzer>::skip_eq_reg,
            &cpu<Screen, Keyboard, Buzzer>::load_imm,
            &cpu<Screen, Keyboard, Buzzer>::add_imm
        };

    static std::unordered_map<uint8_t, operation> first_8 =
        {   {0x00, &cpu<Screen, Keyboard, Buzzer>::load_reg},
            {0x01, &cpu<Screen, Keyboard, Buzzer>::or_reg},
            {0x02, &cpu<Screen, Keyboard, Buzzer>::and_reg},
            {0x03, &cpu<Screen, Keyboard, Buzzer>::xor_reg},
            {0x04, &cpu<Screen, Keyboard, Buzzer>::add_reg},
            {0x05, &cpu<Screen, Keyboard, Buzzer>::sub_reg},
            {0x06, &cpu<Screen, Keyboard, Buzzer>::shr},
            {0x07, &cpu<Screen, Keyboard, Buzzer>::subn},
            {0x0E, &cpu<Screen, Keyboard, Buzzer>::shl}
        };

    static std::vector<operation> first_9_to_13 =
        {   &cpu<Screen, Keyboard, Buzzer>::skip_neq_reg,
            &cpu<Screen, Keyboard, Buzzer>::load_addr,
            &cpu<Screen, Keyboard, Buzzer>::jump_addr,
            &cpu<Screen, Keyboard, Buzzer>::rnd,
            &cpu<Screen, Keyboard, Buzzer>::draw
        };

    static std::unordered_map<uint8_t, operation> first_15 =
        {   {0x07, &cpu<Screen, Keyboard, Buzzer>::load_delay},
            {0x0A, &cpu<Screen, Keyboard, Buzzer>::load_key},
            {0x15, &cpu<Screen, Keyboard, Buzzer>::set_delay},
            {0x18, &cpu<Screen, Keyboard, Buzzer>::set_sound},
            {0x1E, &cpu<Screen, Keyboard, Buzzer>::add_i},
            {0x29, &cpu<Screen, Keyboard, Buzzer>::load_sprite},
            {0x33, &cpu<Screen, Keyboard, Buzzer>::load_bcd},
            {0x55, &cpu<Screen, Keyboard, Buzzer>::copy_reg},
            {0x65, &cpu<Screen, Keyboard, Buzzer>::read_reg},
        };

    uint8_t start_number = first_nibble(opcode);

    if(start_number >= 1 && start_number <= 7)
    {
        return first_1_to_7[start_number - 1];
    }

    if(start_number >= 9 && start_number <= 13)
    {
        return first_9_to_13[start_number - 9];
    }

    if(start_number == 8)
    {
        return first_8[fourth_nibble(opcode)];
    }

    uint8_t last_byte = lowest_byte(opcode);

    if(start_number == 15)
    {
        return first_15[last_byte];
    }

    if(start_number == 0)
    {
        if(last_byte == 0xE0)
        {
            return &cpu<Screen, Keyboard, Buzzer>::clear;
        }
        else if(last_byte == 0xEE)
        {
            return &cpu<Screen, Keyboard, Buzzer>::ret;
        }
        else
        {
            return &cpu<Screen, Keyboard, Buzzer>::call_machine;
        }
    }

    // starting with E (14)

    if(last_byte == 0x9E)
    {
        return &cpu<Screen, Keyboard, Buzzer>::skip_eq_key;
    }

    return &cpu<Screen, Keyboard, Buzzer>::skip_neq_key;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::call_machine(uint16_t)
{
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::clear(uint16_t)
{
    screen_clear_proxy.set();

    screen_clear_proxy.wait_for_call();
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::ret(uint16_t)
{
    PC = mem.get_dword(SP);
    SP -= 2;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::jump(uint16_t opcode)
{
    PC = lowest_12_bits(opcode) - 2;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::call_sub(uint16_t opcode)
{
    mem.set_dword(PC, SP += 2);
    jump(opcode);
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::skip_eq_imm(uint16_t opcode)
{
    auto [reg_idx, byte] = second_nibble_byte(opcode);

    if(reg[reg_idx] == byte)
    {
        PC += 2;
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::skip_neq_imm(uint16_t opcode)
{
    auto [reg_idx, byte] = second_nibble_byte(opcode);

    if(reg[reg_idx] != byte)
    {
        PC += 2;
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::skip_eq_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    if(reg[reg_idx_1] == reg[reg_idx_2])
    {
        PC += 2;
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::load_imm(uint16_t opcode)
{
    auto [reg_idx, byte] = second_nibble_byte(opcode);

    reg[reg_idx] = byte;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::add_imm(uint16_t opcode)
{
    auto [reg_idx, byte] = second_nibble_byte(opcode);

    reg[reg_idx] += byte;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::load_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    reg[reg_idx_1] = reg[reg_idx_2];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::or_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    reg[reg_idx_1] |= reg[reg_idx_2];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::and_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    reg[reg_idx_1] &= reg[reg_idx_2];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::xor_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    reg[reg_idx_1] ^= reg[reg_idx_2];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::add_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    uint16_t result = reg[reg_idx_1] + reg[reg_idx_2];

    uint8_t flag = (result > 255) ? 1 : 0;

    reg[reg_idx_1] = lowest_byte(result);

    reg_F = flag;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::sub_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    uint8_t flag = (reg[reg_idx_2] > reg[reg_idx_1]) ? 0 : 1;

    reg[reg_idx_1] -= reg[reg_idx_2];

    reg_F = flag;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::shr(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    reg[reg_idx_1] = reg[reg_idx_2];

    uint8_t flag = lsb(reg[reg_idx_1]);

    reg[reg_idx_1] >>= 1;

    reg_F = flag;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::subn(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    uint8_t flag = (reg[reg_idx_2] < reg[reg_idx_1]) ? 0 : 1;

    reg[reg_idx_1] = reg[reg_idx_2] - reg[reg_idx_1];

    reg_F = flag;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::shl(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    reg[reg_idx_1] = reg[reg_idx_2];

    uint8_t flag = msb(reg[reg_idx_1]);

    reg[reg_idx_1] <<= 1;

    reg_F = flag;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::skip_neq_reg(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2] = middle_nibbles(opcode);

    if(reg[reg_idx_1] != reg[reg_idx_2])
    {
        PC += 2;
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::load_addr(uint16_t opcode)
{
    reg_I = lowest_12_bits(opcode);
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::jump_addr(uint16_t opcode)
{
    PC = lowest_12_bits(opcode) + reg[0];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::rnd(uint16_t opcode)
{
    auto [reg_idx, byte] = second_nibble_byte(opcode);

    reg[reg_idx] = dist_255(rng) & byte;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::draw(uint16_t opcode)
{
    auto [reg_idx_1, reg_idx_2, num_bytes] = three_nibbles_from_second(opcode);

    screen_draw_proxy.set(reg[reg_idx_1], reg[reg_idx_2], mem.bytes(reg_I, num_bytes));

    screen_draw_proxy.wait_for_call();

    reg_F = screen_draw_proxy.get();
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::skip_eq_key(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    keyboard_key_pressed_proxy.set(reg[reg_idx]);

    keyboard_key_pressed_proxy.wait_for_call();

    if(keyboard_key_pressed_proxy.get())
    {
        PC += 2;
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::skip_neq_key(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    keyboard_key_pressed_proxy.set(reg[reg_idx]);

    keyboard_key_pressed_proxy.wait_for_call();

    if(!keyboard_key_pressed_proxy.get())
    {
        PC += 2;
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::load_delay(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    reg[reg_idx] = delay_timer;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::load_key(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    keyboard_wait_for_key_pressed_proxy.set();

    keyboard_wait_for_key_pressed_proxy.wait_for_call();

    reg[reg_idx] = keyboard_wait_for_key_pressed_proxy.get();
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::set_delay(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    delay_timer = reg[reg_idx];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::set_sound(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    sound_timer = reg[reg_idx];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::add_i(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    reg_I += reg[reg_idx];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::load_sprite(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    reg_I += reg[reg_idx];
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::load_bcd(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    auto value = reg[reg_idx];

    mem[reg_I]     = value / 100;
    mem[reg_I + 1] = (value / 10) % 10;
    mem[reg_I + 2] = value % 10;
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::copy_reg(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    for(uint16_t offset = 0; offset <= reg_idx; offset++)
    {
        mem[reg_I + offset] = reg[offset];
    }
}

template<typename Screen, typename Keyboard, typename Buzzer>
void cpu<Screen, Keyboard, Buzzer>::read_reg(uint16_t opcode)
{
    auto reg_idx = second_nibble(opcode);

    for(uint16_t offset = 0; offset <= reg_idx; offset++)
    {
        reg[offset] = mem[reg_I + offset];
    }
}
