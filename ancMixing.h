#ifndef ANC_MIXING_H
#define ANC_MIXING_H

#include <algorithm>
#include <cstdint> // Include this for uint8_t

struct Gains {
    float attention = 0.50f / 100.0f;
    float meditation = -0.1f / 100.0f;
    float heartrate = 0.3f / 180.0f;
    float blood_oxygen = 0.125f / 100.0f;
    float rms = 0.75f / 80.0f;
    float new_mix_influence = 0.2f;
};

struct Offsets {
    float attention = 0.0f;
    float meditation = 0.0f;
    float heartrate = 0.0f;
    float blood_oxygen =  96.0f;
    float rms = 50.0f;
    float new_mix_influence = 0.0f;
};

class AncMixing {
public:
    float current_mix;
    Gains gains;
    Offsets offsets;

    AncMixing(const Gains& gains);
    float update(uint8_t attention, uint8_t meditation, float heartrate, float blood_oxygen, float rms);
};

#endif // ANC_MIXING_H
