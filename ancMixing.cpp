#include "ancMixing.h"
#include <cstdint> // Include this for uint8_t
#include <iostream>

AncMixing::AncMixing(const Gains& gains) : current_mix(0), gains(gains) {}

float map_clamp(float input, float min, float max, float range) {
    float t = (input - min) / (max - min);

    if (t > 1) {
        t = 1;
    } else if (t < 0) {
        t = 0;
    }

    // std::cout << input << " " << t << std::endl;

    if (range < 0) {
        return (1 - t) * range;
    } else {
        return t * range;
    }
}

float AncMixing::update(uint8_t attention, uint8_t meditation, float heartrate, float blood_oxygen, float rms) {
    float new_mix = 0.0f;
    new_mix += map_clamp(attention, offsets.attention, 100, gains.attention); // * (attention - offsets.attention);
    new_mix += map_clamp(meditation, offsets.meditation, 100, gains.meditation); // * (meditation - offsets.meditation);
    new_mix += map_clamp(heartrate, offsets.heartrate, 180, gains.heartrate); // * (heartrate - offsets.heartrate);
    new_mix += map_clamp(blood_oxygen, offsets.blood_oxygen, 100, gains.blood_oxygen); // * (blood_oxygen - offsets.blood_oxygen);
    new_mix += map_clamp(rms, offsets.rms, 80, gains.rms); // * (rms - offsets.rms);

    new_mix = std::min(std::max(new_mix, 0.0f), 1.0f);
    current_mix = gains.new_mix_influence * new_mix + (1.0f - gains.new_mix_influence) * current_mix;

    return current_mix;
}
