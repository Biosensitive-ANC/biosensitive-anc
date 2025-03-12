#ifndef WHITE_NOISE_H
#define WHITE_NOISE_H

#include <alsa/asoundlib.h>
#include <cmath>
#include <vector>
#include "ancMixing.h"

void sensorDisplayThread(SharedData* shared, EEGSerial* eeg, RecUart* recUart);
void audioThread(SharedData* shared, AncMixing* mixer);

#endif // WHITE_NOISE_H
