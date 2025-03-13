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
#include "common.h"


// Compute RMS value of the captured audio buffer
float computeRMS(const fixed_sample_type* buffer, int size) {
    static float average;
    const float K = 0.9f;
    float sum = 0.0f;

    for (int i = 0; i < size; i++) {
        sum += (float) buffer[i] * (float) buffer[i];  // Square each sample
    }

    sum /= 100000000.f;

    average = K * average + ((1-K) * sum/size);
    return std::log10(std::sqrt(average)) * 20;
}

// Pre-generate white noise samples
void generateWhiteNoise(fixed_sample_type* buffer, int size, float mixLevel) {
    for (int i = 0; i < size; i++) {
        fixed_sample_type sample = ((std::rand() % std::numeric_limits<fixed_sample_type>::max()) - (std::numeric_limits<fixed_sample_type>::max() >> 1)) * mixLevel;
        buffer[i] = std::max(std::min(sample, std::numeric_limits<fixed_sample_type>::max()), std::numeric_limits<fixed_sample_type>::min());
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
        eeg->getData(&attention, &meditation);
        recUart->getData(&bpm, &spo2);

        // Update shared data with mutex protection
        {
            // std::lock_guard<std::mutex> lock(shared->dataMutex);
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
    std::cout << "ENDED OLED AAAAAAA" << std::endl;
}

void audioThread(SharedData* shared, AncMixing* mixer) {
    snd_pcm_uframes_t frames_per_period = FRAMES_PER_PERIOD;
    snd_pcm_uframes_t frames_per_device_buffer = PERIODS_PER_BUFFER * FRAMES_PER_PERIOD;

    snd_pcm_t *cap_handle;
    init_capture(&cap_handle, PLAY_FREQ, &frames_per_period, &frames_per_device_buffer, NR_OF_CHANNELS, "hw:3,1");

    snd_pcm_t *play_handle;
    init_playback(&play_handle, PLAY_FREQ, &frames_per_period, &frames_per_device_buffer, NR_OF_CHANNELS, "default");

    std::array<fixed_sample_type, BUFFER_SAMPLE_SIZE> capture_buffer = {0};
    std::array<fixed_sample_type, BUFFER_SAMPLE_SIZE> process_buffer = {0};
    std::array<fixed_sample_type, BUFFER_SAMPLE_SIZE> playback_buffer = {0};

    while (shared->running) {
        // Create threads for capture, processing, and playback
        std::thread capture_thread([&] {
            capture(cap_handle, capture_buffer.data(), FRAMES_PER_PERIOD);
        });
        
        // playback might not need its own thread , not sure if it is blocking
        // look at is block transfer = ??? in runtime
        std::thread playback_thread([&] {
            playback(play_handle, playback_buffer.data(), FRAMES_PER_PERIOD);
        });

        // std::cout << "Buf " << capture_buffer[0] << std::endl;
        // std::cout << "Proc Buf " << process_buffer[0] << std::endl;
        // std::cout << "Play Buf " << playback_buffer[0] << std::endl;
        shared->rms = computeRMS(capture_buffer.data(), BUFFER_SAMPLE_SIZE);

        float mix = mixer->update(shared->attention, shared->meditation, shared->bpm, shared->spo2, shared->rms);
        
        #ifdef WHITE_NOISE_IMPLEMENTATION
        generateWhiteNoise(process_buffer.data(), BUFFER_SAMPLE_SIZE, mix); // idk if the mix level will update properly
        #else
        const fixed_sample_type multiplier = 8;
        const fixed_sample_type MAXVAL = std::numeric_limits<fixed_sample_type>::max() / 8;
        const fixed_sample_type MINVAL = std::numeric_limits<fixed_sample_type>::min() / 8;
        std::transform(process_buffer.begin(), process_buffer.end(), process_buffer.begin(),
                [multiplier](fixed_sample_type& val) { 
                    if(val > MAXVAL)
                        return std::numeric_limits<fixed_sample_type>::max();
                    if(val < MINVAL) 
                        return std::numeric_limits<fixed_sample_type>::min();
                    return multiplier * val;
                });
        #endif

        // Wait for all threads to complete
        capture_thread.join();
        playback_thread.join();

        // Exchange data between buffers (must be done after all threads complete)
        std::copy(process_buffer.begin(), process_buffer.end(), playback_buffer.begin());
        std::copy(capture_buffer.begin(), capture_buffer.end(), process_buffer.begin());
    }
    
    snd_pcm_drain(play_handle);
    snd_pcm_close(play_handle);
}
