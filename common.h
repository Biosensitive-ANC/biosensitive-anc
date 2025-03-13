#include <mutex>
#include <atomic>

#ifndef COMMON_H
#define COMMON_H

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

#endif