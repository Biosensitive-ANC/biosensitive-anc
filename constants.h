
//
// Created by pitersk on 23.10.18.
//

#ifndef RPIANC_CONSTANTS_H
#define RPIANC_CONSTANTS_H

typedef float sample_type;
typedef int32_t fixed_sample_type;

const std::string RPI_CAPTURE_DEVICE_NAME = "hw:3,1";
const std::string RPI_PLAYBACK_DEVICE_NAME = "default";
const size_t NR_OF_CHANNELS = 2;

const size_t FRAMES_PER_PERIOD = 32;
const size_t PERIODS_PER_BUFFER = 32;
const size_t BUFFER_SAMPLE_SIZE = FRAMES_PER_PERIOD*NR_OF_CHANNELS;


#endif //RPIANC_CONSTANTS_H
