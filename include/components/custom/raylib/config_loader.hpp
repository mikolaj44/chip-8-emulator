#pragma once

#include <filesystem>
#include <variant>

#include "components/core/cpu.hpp"
#include "components/custom/raylib/screen.hpp"
#include "components/custom/raylib/buzzer.hpp"
#include "components/custom/raylib/keyboard.hpp"
#include "utils/memory_loaders.hpp"

#include <nlohmann/json.hpp>

namespace
{
    class error_message
    {
    public:
        void add_message(const std::string& message)
        {
            full_message += message + "\n";

            count++;
        }

        std::string get_full_message()
        {
            return std::format("Number of config JSON errors: {}\n", count) + full_message;
        }

        size_t count = 0;

    private:
        std::string full_message;
    };
}

std::variant<std::string, cpu<screen, keyboard, buzzer>> create_cpu_from_config(const std::string& json_name, const std::string& rom_path = "")
{
    using namespace nlohmann;
    using namespace nlohmann::detail;

    std::fstream filestream;
    std::string file_open_message = "Could not open the JSON config file, make sure that the path is correct";

    try
    {
        filestream.open(json_name);
    }
    catch (const std::exception& ex) {
        return std::format("{}, got exception:\n{}", file_open_message, ex.what());
    }

    if(filestream.fail())
    {
        return std::format("{}, reason: fstream.open failed", file_open_message);
    }

    json data;

    try
    {
        data = json::parse(filestream);
    }
    catch (const std::exception& ex) {
        return std::format("Could not parse the JSON config file, make sure that it is in the JSON format, got exception:\n{}", ex.what());
    }

    // program
    auto read_rom_path = data["program"]["rom_path"];
    auto start_byte = data["program"]["start_byte"];

    // screen
    auto on_r = data["screen"]["on_color"]["r"];
    auto on_g = data["screen"]["on_color"]["g"];
    auto on_b = data["screen"]["on_color"]["b"];

    auto off_r = data["screen"]["off_color"]["r"];
    auto off_g = data["screen"]["off_color"]["g"];
    auto off_b = data["screen"]["off_color"]["b"];

    auto pixel_size = data["screen"]["pixel_size"];
    auto screen_frequency = data["screen"]["frequency_hz"];

    // cpu
    auto cpu_frequency = data["cpu"]["frequency_hz"];

    // timers
    auto timers_frequency = data["timers"]["frequency_hz"];

    // buzzer
    auto buzzer_frequency = data["buzzer"]["frequency_hz"];
    auto buzzer_volume = data["buzzer"]["volume"];

    bool read_rom_path_provided = (read_rom_path.type() == value_t::string);
    bool rom_path_provided = (rom_path != "");

    error_message message;

    // command line ROM path has higher precedence
    std::string actual_rom_path = "";

    if(!read_rom_path_provided && !rom_path_provided)
    {
        message.add_message("ROM path string not provided in neither the command line arugment nor the JSON config");
    }
    else if(rom_path_provided && std::filesystem::exists(rom_path))
    {
        actual_rom_path = rom_path;
    }
    else if(read_rom_path_provided && std::filesystem::exists(read_rom_path))
    {
        actual_rom_path = read_rom_path;
    }

    if(actual_rom_path == "")
    {
        message.add_message("provided ROM path (command-line or JSON config) does not point to a file");
    }

    if(start_byte.type() == value_t::number_unsigned && (start_byte < 0 || start_byte > 4095))
    {
        message.add_message("program.start_byte needs to be an integer in range [0, 4095]");
    }

    bool valid_on_color = false;
    bool valid_off_color = false;

    auto check_color = [&](bool is_on)
    {
        std::array<json::value_type, 3> color_members = {on_r, on_g, on_b};
        std::array<char, 3> raw_color_members = {'r', 'g', 'b'};
        std::string on_str = "on";

        if(!is_on)
        {
            color_members = {off_r, off_g, off_b};
            on_str = "off";
        }

        bool issues_found = false;

        for(size_t i = 0; i < color_members.size(); i++)
        {
            const auto& member = color_members[i];
            const auto& raw_member_name = raw_color_members[i];

            if(member.type() == value_t::number_unsigned && (member < 0 || member > 255))
            {
                message.add_message(std::format("screen.{}_color.{} needs to be an integer in range [0, 255]", on_str, raw_member_name));

                issues_found = true;
            }
        }

        if(!issues_found)
        {
            if(is_on)
            {
                valid_on_color = true;
            }
            else
            {
                valid_off_color = true;
            }
        }
    };

    check_color(true);
    check_color(false);

    if(pixel_size.type() == value_t::number_unsigned && pixel_size <= 0)
    {
        message.add_message("screen.pixel_size needs to be a positive integer");
    }

    if(!screen_frequency.is_null() && screen_frequency.type() != value_t::number_unsigned)
    {
        message.add_message("screen.frequency_hz needs to be a non-negative integer");
    }

    if(!cpu_frequency.is_null() && cpu_frequency.type() != value_t::number_unsigned)
    {
        message.add_message("cpu.frequency_hz needs to be a non-negative integer");
    }

    if(!timers_frequency.is_null() && timers_frequency.type() != value_t::number_unsigned)
    {
        message.add_message("timers.frequency_hz needs to be a non-negative integer");
    }

    if(!buzzer_frequency.is_null() && buzzer_frequency <= 0)
    {
        message.add_message("buzzer.frequency_hz needs to be a positive integer");
    }

    if(!buzzer_volume.is_null() && (buzzer_volume.type() != value_t::number_float || buzzer_volume.type() != value_t::number_integer || buzzer_volume.type() != value_t::number_unsigned) && buzzer_volume < 0)
    {
        message.add_message("buzzer.volume needs to be a non-negative number (float or int)");
    }

    if(message.count > 0)
    {
        return message.get_full_message();
    }

    memory mem{start_byte.is_null() ? memory::default_program_start_byte : static_cast<uint16_t>(start_byte)};

    load_rom_from_path(mem, actual_rom_path);

    keyboard keyb;

    screen scr{(pixel_size.is_null() ? screen::default_pixel_size : static_cast<size_t>(pixel_size)),
                (valid_on_color ? Color{static_cast<unsigned char>(on_r), static_cast<unsigned char>(on_g), static_cast<unsigned char>(on_b), 255} : screen::default_on_color),
                (valid_off_color ? Color{static_cast<unsigned char>(off_r), static_cast<unsigned char>(off_g), static_cast<unsigned char>(off_b), 255} : screen::default_off_color)};

    buzzer buzz{buzzer_frequency.is_null() ? buzzer::default_frequency : static_cast<size_t>(buzzer_frequency),
                buzzer_volume.is_null() ? buzzer::default_volume : static_cast<float>(buzzer_volume)};

    using cpu = cpu<screen, keyboard, buzzer>;

    cpu processor{mem, scr, keyb, buzz,
        cpu_frequency.is_null() ? cpu::default_cpu_frequency_hz : static_cast<size_t>(cpu_frequency),
        timers_frequency.is_null() ? cpu::default_timers_frequency_hz : static_cast<size_t>(timers_frequency),
        screen_frequency.is_null() ? cpu::default_screen_frequency_hz : static_cast<size_t>(screen_frequency),
    };

    return processor;
}
