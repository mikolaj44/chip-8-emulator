#include <iostream>

#include "components/custom/raylib/config_loader.hpp"

constexpr char const* CONFIG_FILE_NAME = "config.json";

int main(int argc, char* argv[])
{
    std::string rom_path = (argc > 1 ? argv[1] : "");

    const auto result = create_cpu_from_config(CONFIG_FILE_NAME, rom_path);

    if(std::holds_alternative<std::string>(result))
    {
        std::cout << std::get<std::string>(result) << "\n";

        return 1;
    }

    auto processor = std::get<cpu<screen, keyboard, buzzer>>(result);

    processor.start();

    std::cin.get();

    return 0;
}
