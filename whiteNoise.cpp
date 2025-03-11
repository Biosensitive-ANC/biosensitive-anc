#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <cmath>
#include <mutex>
#include <atomic>
#include <chrono>
#include <array>
#include "whiteNoise.h"
#include "oled.h"
#include "i2c_manager.h"
#include "EEG.h"
#include "rec_uart.h"
#include "constants.h"
#include "capture.h"
#include "playback.h"

#define SAMPLE_RATE 48000
#define CHANNELS 2
#define FRAME_SIZE (CHANNELS * sizeof(int16_t))
#define BUFFER_FRAMES 1024
#define OLED_UPDATE_MS 1000 // Update OLED every 250ms seems to be okay

// Shared data between threads
struct SharedData {
    std::mutex dataMutex;
    uint8_t attention = 0;
    uint8_t meditation = 0;
    float bpm = 0.0f;
    float spo2 = 0.0f;
    float rms = 0.0f;
    float mix_level = 0.0f;
    std::atomic<bool> running{true};
};

// Compute RMS value of the captured audio buffer
float computeRMS(const int16_t* buffer, int size) {
    static float average;
    const float K = 0.9f;
    float sum = 0.0f;

    for (int i = 0; i < size; i++) {
        sum += buffer[i] * buffer[i];  // Square each sample
    }

    average = K * average + ((1-K) * sum/size);
    return std::log10(std::sqrt(average)) * 20;
}

// Pre-generate white noise samples
void generateWhiteNoise(int16_t* buffer, int size, float mixLevel) {
    for (int i = 0; i < size; i++) {
        int32_t sample = ((std::rand() % 65536) - 32768) * mixLevel;
        buffer[i] = std::max(std::min(sample, (int32_t)INT16_MAX), (int32_t)INT16_MIN);
    }
}

// Thread function for handling sensors and OLED display
void sensorDisplayThread(SharedData* shared, EEGSerial* eeg, RecUart* recUart) {
    OLED_Init();
    OLED_Clear();
    OLED_ShowString(0, 0, "Attention: ");
    OLED_ShowString(0, 2, "Meditation: ");
    OLED_ShowString(0, 4, "bpm: ");
    OLED_ShowString(0, 6, "spo2: ");
    
    while (shared->running) {
        uint8_t attention, meditation, bpm;
        float spo2;
        
        // Read sensor data
        eeg->getData(attention, meditation);
        recUart->getData(bpm, spo2);
        
        // Update shared data with mutex protection
        {
            std::lock_guard<std::mutex> lock(shared->dataMutex);
            shared->attention = attention;
            shared->meditation = meditation;
            shared->bpm = bpm;
            shared->spo2 = spo2;
        }
        
        // Update OLED display
        OLED_ShowUInt8_twochar(88, 0, attention);
        OLED_ShowUInt8_twochar(96, 2, meditation);
        OLED_ShowUInt8_threechar(40, 4, bpm);
        OLED_ShowFloat(48, 6, spo2);
        
        // Sleep to avoid excessive OLED updates
        std::this_thread::sleep_for(std::chrono::milliseconds(OLED_UPDATE_MS));
    }
}

void audioThread(SharedData* shared, AncMixing* mixer) {
    unsigned int play_freq = 48000;
    unsigned int number_of_channels = NR_OF_CHANNELS;
    snd_pcm_uframes_t frames_per_period = FRAMES_PER_PERIOD;
    snd_pcm_uframes_t frames_per_device_buffer = PERIODS_PER_BUFFER * FRAMES_PER_PERIOD;

    snd_pcm_t *cap_handle;
    init_capture(&cap_handle, &play_freq, &frames_per_period, &frames_per_device_buffer, NR_OF_CHANNELS, "hw:3,1");

    snd_pcm_t *play_handle;
    init_playback(&play_handle, &play_freq, &frames_per_period, &frames_per_device_buffer, NR_OF_CHANNELS, "default");

    std::array<fixed_sample_type, BUFFER_SAMPLE_SIZE> capture_buffer = {0};
    std::array<fixed_sample_type, BUFFER_SAMPLE_SIZE> playback_buffer = {0};

    while (shared->running) {
        // Create threads for capture, processing, and playback
        std::thread capture_thread([&] {
            capture(cap_handle, capture_buffer.data(), FRAMES_PER_PERIOD);

            const fixed_sample_type multiplier = 8;
            std::transform(capture_buffer.begin(), capture_buffer.end(), capture_buffer.begin(),
                   [multiplier](fixed_sample_type& val) { 
                    fixed_sample_type retval = multiplier * val;
                        if(retval > std::numeric_limits<fixed_sample_type>::max())
                            return std::numeric_limits<fixed_sample_type>::max();
                        if(retval < std::numeric_limits<fixed_sample_type>::min()) 
                            return std::numeric_limits<fixed_sample_type>::min();
                        return retval;
                    });
        });

        std::thread playback_thread([&] {
            playback(play_handle, playback_buffer.data(), FRAMES_PER_PERIOD);
        });

        // Wait for all threads to complete
        capture_thread.join();
        playback_thread.join();

        // Exchange data between buffers (must be done after all threads complete)
        std::copy(capture_buffer.begin(), capture_buffer.end(), playback_buffer.begin());
    }
    
    snd_pcm_drain(play_handle);
    snd_pcm_close(play_handle);
}

void playWhiteNoise(AncMixing& mixer) {
    // Initialize shared data structure
    SharedData shared;
    
    // Initialize sensors
    EEGSerial eeg;
    RecUart recUart;
    
    // Create threads
    std::thread audio_thread(audioThread, &shared, &mixer);

    sensorDisplayThread(&shared, &eeg, &recUart);
    
    // Wait for threads to finish (use Ctrl+C to exit)
    if (audio_thread.joinable()) audio_thread.join();
}
