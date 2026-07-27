#include "components/core/cpu.hpp"
#include "components/custom/example/screen.hpp"
#include "components/custom/example/keyboard.hpp"
#include "components/custom/example/buzzer.hpp"
#include "utils/memory_loaders.hpp"

constexpr uint16_t PROGRAM_START_BYTE = 512;
constexpr size_t CPU_FREQUENCY_HZ = 800;
constexpr size_t TIMERS_FREQUENCY_HZ = 60;
constexpr size_t SCREEN_FREQUENCY_HZ = 60;

int main()
{
    screen scr{};
    keyboard keyb{};
    buzzer buzz{};

    memory mem{PROGRAM_START_BYTE};

    load_example_rom(mem);

    cpu<screen, keyboard, buzzer> processor{mem, scr, keyb, buzz, CPU_FREQUENCY_HZ, TIMERS_FREQUENCY_HZ, SCREEN_FREQUENCY_HZ};

    processor.start();

    std::cin.get();

    return 0;
}
