#ifndef WHITE_NOISE_H
#define WHITE_NOISE_H

#include <alsa/asoundlib.h>
#include <cmath>
#include <vector>
#include "ancMixing.h"
#include "common.h"
#include "EEG.h"
#include "rec_uart.h"

void sensorDisplayThread(SharedData* shared, EEGSerial* eeg, RecUart* recUart);
void audioThread(SharedData* shared, AncMixing* mixer);

#endif // WHITE_NOISE_H
