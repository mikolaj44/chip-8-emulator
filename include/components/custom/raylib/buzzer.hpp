#pragma once

#include <cmath>
#include <array>

#include <raylib.h>

// this class depends on the raylib window being already initialized
// not pretty but avoids a headache

// https://www.raylib.com/examples/audio/loader.html?name=audio_raw_stream
class buzzer
{
public:
    buzzer(size_t frequency = default_frequency, float volume = default_volume)
        : frequency(frequency), volume(volume) {};

    buzzer(const buzzer& other)
        : buzzer(other.frequency, other.volume) {};

    void setup()
    {
        InitAudioDevice();

        SetAudioStreamBufferSizeDefault(BUFFER_SIZE);
        SetMasterVolume(volume);

        stream = LoadAudioStream(SAMPLE_RATE, 32, 1);

        PlayAudioStream(stream);

        audio_device_initialized = true;

        stop();
    }

    void start()
    {
        is_stopped = false;

        ResumeAudioStream(stream);
    }

    void stop()
    {
        is_stopped = true;

        PauseAudioStream(stream);
    }

    void update()
    {
        if (IsAudioStreamProcessed(stream))
        {
            for (size_t i = 0; i < BUFFER_SIZE; i++)
            {
                size_t wavelength = SAMPLE_RATE / frequency;
                buffer[i] = sin(2 * PI * sine_index / wavelength);

                sine_index++;

                if (sine_index >= wavelength)
                {
                    sine_index = 0;
                }
            }

            UpdateAudioStream(stream, buffer.data(), BUFFER_SIZE);
        }
    }

    static constexpr size_t default_frequency = 440;
    static constexpr float default_volume = 0.1f;

private:
    AudioStream stream;

    bool is_stopped = true;
    bool audio_device_initialized = false;

    const size_t frequency;
    const float volume;

    static constexpr size_t BUFFER_SIZE = 4096;
    static constexpr size_t SAMPLE_RATE = 44100;

    size_t sine_index = 0;

    std::array<float, BUFFER_SIZE> buffer{};
};
