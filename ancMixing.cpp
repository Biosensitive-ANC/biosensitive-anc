#include "ancMixing.h"
#include <cstdint> // Include this for uint8_t

AncMixing::AncMixing(const Gains& gains) : current_mix(0), gains(gains) {}

float AncMixing::update(uint8_t attention, uint8_t meditation, float heartrate, float blood_oxygen, float rms) {
    float new_mix = 0.0f;
    new_mix += gains.attention * (attention - offsets.attention);
    new_mix += gains.meditation * (meditation - offsets.meditation);
    new_mix += gains.heartrate * (heartrate - offsets.heartrate);
    new_mix += gains.blood_oxygen * (blood_oxygen - offsets.blood_oxygen);
    new_mix += gains.rms * (rms - offsets.rms);

    new_mix = std::min(std::max(new_mix, 0.0f), 1.0f);
    current_mix = (1.0f - gains.new_mix_influence) * new_mix + gains.new_mix_influence * current_mix;

    return current_mix;
}
