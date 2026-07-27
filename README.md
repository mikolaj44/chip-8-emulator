# CHIP-8 Emulator

This is my first, simple emulator project for the original CHIP-8 fantasy console. This is also my first time using raylib (also I didn't use any LLMs for anything)

Sources used:

- [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM)
- [CHIP-8 Variant Opcode Table](https://chip8.gulrak.net)
- [CHIP-8 - Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
- [This awesome test suite from Timendus](https://github.com/Timendus/chip8-test-suite)
- [Pong game from kripod's collection](https://github.com/kripod/chip8-roms)

The project can be used as a standalone Linux/Windows app (haven't tested MacOS but should work too) or as a base on top of which custom components can be easily added, for example to make it work on a Raspberry PI Pico with some real LCD screen.

Only prerequisites should be CMake >= 3.13 and standard build tools for your system.

## Building the project

A target must be chosen at CMake configure stage by passing it as a variable via `-DTARGET_NAME`:

- `raylib` for the standalone application using [raylib](https://github.com/raysan5/raylib) and [json](https://github.com/nlohmann/json) - these dependencies will be automatically installed with FetchContent
- `example` for the application that doesn't use any additional dependencies, that can be used as a base for adding custom components
- `all` for the two above

**Note**: this target can't be left empty.

```bash
cd path/to/chip-8-emulator
mkdir build
cmake -B build -DTARGET_NAME=[raylib|example|all]
cmake --build build
```

There are also builds available for Linux and Windows in *Releases* that can be run without installing any dependencies.

## Running the standalone emulator app

There must be a configuration file called `config.json` located in **your working directory**, the ROM path here is also relative to it:

```json
{
    "program": {
        "rom_path": "path/to/roms/some-rom.rom",
        "start_byte": 512
    },
    "screen": {
        "on_color": {
            "r": 255,
            "g": 255,
            "b": 255
        },
        "off_color": {
            "r": 0,
            "g": 0,
            "b": 0
        },
        "pixel_size": 16,
        "frequency_hz": 60
    },
    "cpu": {
        "frequency_hz": 1000
    },
    "timers":
    {
        "frequency_hz": 60
    },
    "buzzer": {
        "frequency_hz": 440,
        "volume": 0.1
    }
}
```

An example config is located at the root of the repo.

You can run the program like so:

```bash
./chip-8
./chip-8 path/to/binary-rom
```

The second option overrides the ROM file provided in the configuration file. If there are any issues with your config, the error message should tell you where the problem is.


## Components

#### Custom components

These are used for visuals, keyboard interaction and audio - all later passed as template parameters to the **cpu**. They need to implement a minimal set of methods, which are listed here, but they can have additional ones, which will be explained later. The barebone versions of the components that have just logging added are under `include/components/custom/example`.

Each of these components is run in a loop and has a thread dedicated to it, so all methods related to some component are isolated **only** to that thread.

The **screen** component is used as a display, for example as a raylib window or for a physical LCD screen.

```cpp
// clear the screen, needed by an opcode
void clear();

// draw sprites, return true if any were erased, also needed by an opcode
bool draw(uint8_t x, uint8_t y, std::vector<uint8_t> bytes)

// update (refresh) the screen, not needed by an opcode but prevents flickering
void update();
```

The **keyboard** component is used for interacting with a user, for example for the PC keyboard or some custom keypad.

```cpp
// checks if a key was pressed, needed by an opcode
bool key_pressed(uint8_t keycode);

// waits for a key to be pressed, blocking operation, also needed by an opcode
uint8_t wait_for_key_pressed();
```

The **buzzer** component is used for playing audio, this is typically a single tone but you can also play a custom sound file for example.

```cpp
// start playing the sound
void start();

// stop playing the sound
void stop()
```

#### Core components

These are build according to the specification the Internet has agreed on over the years, so they shouldn't be modified, but you are free to do so if you want to.

The **memory** is a wrapper for a 4096-byte array with the default sprites preloaded and some utilities:

```cpp
memory(uint16_t program_start_byte = 512)
```

It has the square bracket operator implemented and the underlying array can be accessed by the `mem` member - that should be everything that's needed for loading the program bytes.

The memory-loading functions in `include/utils/memory_loaders.hpp` use this `program_start_byte` as a starting byte index for where to start writing the program data, it's typically 512.


The **cpu** brings all of the previous components together:

```cpp
template<typename Screen, typename Keyboard, typename Buzzer>
cpu(memory, Screen, Keyboard, Buzzer,
    size_t cpu_frequency_hz = 1000,
    size_t timers_frequency_hz = 60,
    size_t screen_frequency_hz = 60);
```

```cpp
void start(); // starts executing instructions (non-blocking)
void stop(); // pauses the execution (non-blockin)
```

The **cpu** also uses two instances of the **timer** class, which is basically just a thread-safe wrapper for an `uint8_t`.

#### Additional methods that the custom components can implement

**Note**: all of the methods described in this section are only called if they exist (with `if constexpr requires`)

When `start()` is called on the **cpu** for the first time, the following methods are called on the components:

```cpp
// you can do initialization here
screen.setup();
keyboard.setup();
buzzer.setup();
```

Then at every `start()` call:

```cpp
// you can resume your threads here for example
screen.start();
keyboard.start();
```

At every iteration of the component thread loop, after `start()` is called:

```cpp
// manipulate your audio buffer for example
keyboard.update();
buzzer.update();
/* note: screen.update() is required and always called
but it's delayed by the refresh rate, other updates are not here */
```

And when `stop()` is called:

```cpp
// for example pausing threads etc.
screen.stop();
keyboard.stop();
// note buzzer.stop() is called here but it's required anyway
```

You can check out how I implemented the raylib components that use those additional methods under `include/components/custom/raylib`, but note that I used a small "hack" there - raylib requires the window to be initialized first etc. and it's also not made for multithreading, but it seems to work anyway.
