#include <iostream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <cmath>
#include <mutex>
#include <atomic>
#include <chrono>
#include "whiteNoise.h"
#include "oled.h"
#include "i2c_manager.h"
#include "EEG.h"
#include "rec_uart.h"

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
    snd_pcm_t *capture_handle, *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    int err;

    // Open I2S capture device
    if ((err = snd_pcm_open(&capture_handle, "hw:3,1", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        fprintf(stderr, "Cannot open input audio device: %s\n", snd_strerror(err));
        shared->running = false;
        return;
    }

    // Open default playback device
    if ((err = snd_pcm_open(&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf(stderr, "Cannot open output audio device: %s\n", snd_strerror(err));
        snd_pcm_close(capture_handle);
        shared->running = false;
        return;
    }

    // Configure capture device with larger buffer
    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_any(capture_handle, hw_params);
    snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(capture_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate(capture_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS);
    snd_pcm_hw_params_set_buffer_size(capture_handle, hw_params, BUFFER_FRAMES * 4); // Larger buffer
    snd_pcm_hw_params_set_period_size(capture_handle, hw_params, BUFFER_FRAMES, 0);
    snd_pcm_hw_params(capture_handle, hw_params);

    // Configure playback device with larger buffer to prevent underruns
    snd_pcm_hw_params_any(playback_handle, hw_params);
    snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_rate(playback_handle, hw_params, SAMPLE_RATE, 0);
    snd_pcm_hw_params_set_channels(playback_handle, hw_params, CHANNELS);
    snd_pcm_hw_params_set_buffer_size(playback_handle, hw_params, BUFFER_FRAMES * 4); // Larger buffer
    snd_pcm_hw_params_set_period_size(playback_handle, hw_params, BUFFER_FRAMES, 0);
    snd_pcm_hw_params(playback_handle, hw_params);

    snd_pcm_hw_params_free(hw_params);
    snd_pcm_prepare(capture_handle);
    snd_pcm_prepare(playback_handle);
    
    int16_t buffer[BUFFER_FRAMES * CHANNELS];
    int16_t noise_buffer[BUFFER_FRAMES * CHANNELS];

    // Set higher priority for audio thread if possible
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        // Fall back to normal priority if SCHED_FIFO not available
        nice(-10); // Try to give this thread a higher priority
    }

    while (shared->running) {
        // Read audio
        memset(buffer, 0, sizeof(buffer));        
        err = snd_pcm_readi(capture_handle, buffer, BUFFER_FRAMES);

        if (err < 0) {
            fprintf(stderr, "Read error: %s\n", snd_strerror(err));
            snd_pcm_prepare(capture_handle);
            continue;
        }

        float rms = computeRMS(buffer, BUFFER_FRAMES * CHANNELS);
        
        // Get current sensor data with mutex protection
        uint8_t attention, meditation;
        float bpm;
        float spo2;
        float mix_level;
        
        {
            std::lock_guard<std::mutex> lock(shared->dataMutex);
            attention = shared->attention;
            meditation = shared->meditation;
            bpm = shared->bpm;
            spo2 = shared->spo2;
            
            // Update the mixer with current values
            mix_level = mixer->update(0, 0, bpm, spo2, rms);
            std::cout << "Mix Level: " << mix_level << std::endl;
            std::cout << "BPM: " << bpm << std::endl;
            std::cout << "SPO2: " << spo2 << std::endl;
            std::cout << "RMS: " << rms << std::endl;
            
            // Save back to shared data
            shared->rms = rms;
            shared->mix_level = mix_level;
        }
        
        // Generate white noise with the current mix level
        generateWhiteNoise(noise_buffer, BUFFER_FRAMES * CHANNELS, mix_level);

        // Write to playback
        err = snd_pcm_writei(playback_handle, buffer, BUFFER_FRAMES);
        if (err < 0) {
            if (err == -EPIPE) {
                fprintf(stderr, "Underrun detected! Recovering...\n");
                snd_pcm_prepare(playback_handle);  // Recover from buffer underrun
            } else {
                fprintf(stderr, "Write error: %s\n", snd_strerror(err));
                snd_pcm_prepare(playback_handle);
            }
        }
    }

    snd_pcm_close(capture_handle);
    snd_pcm_close(playback_handle);
}

void playWhiteNoise(AncMixing& mixer) {
    // Initialize shared data structure
    SharedData shared;
    
    // Initialize sensors
    EEGSerial eeg;
    RecUart recUart;
    
    // Create threads
    std::thread display_thread(sensorDisplayThread, &shared, &eeg, &recUart);
    std::thread audio_thread(audioThread, &shared, &mixer);
    
    // Set main thread to lower priority
    nice(10);
    
    // Wait for threads to finish (use Ctrl+C to exit)
    if (audio_thread.joinable()) audio_thread.join();
    if (display_thread.joinable()) display_thread.join();
}
