#ifndef WHITE_NOISE_H
#define WHITE_NOISE_H

#include <alsa/asoundlib.h>
#include <cmath>
#include <vector>
#include "ancMixing.h"

void playWhiteNoise(AncMixing& mixer);

// Function prototype for computeRMS
float computeRMS(const int16_t* buffer, int size);

void applyGain(int16_t *buffer, size_t samples, float gain);

#endif // WHITE_NOISE_H
