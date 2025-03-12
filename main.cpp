#include <iostream>
#include <thread>
#include "ancMixing.h"
#include "whiteNoise.h"
#include "common.h"


int main() {
    std::cout << "Playing adjustable white noise..." << std::endl;

    Gains gains;
    AncMixing mixer(gains);

    // Initialize shared data structure
    SharedData shared;
    
    // Initialize sensors
    EEGSerial eeg;
    RecUart recUart;
    
    // Create threads
    std::thread audio_thread(audioThread, &shared, &mixer);

    // run the sensor display on the main thread (we dont have enough to make it its own since the audio uses 3 for the pipeline)
    sensorDisplayThread(&shared, &eeg, &recUart);
    
    // Wait for threads to finish (use Ctrl+C to exit)
    if (audio_thread.joinable()) audio_thread.join();
    return 0;
}
