#include <iostream>
#include <thread>
#include "ancMixing.h"
#include "whiteNoise.h"

int main() {
    std::cout << "Playing adjustable white noise..." << std::endl;

    Gains gains;
    AncMixing mixer(gains);

    std::thread audio_thread(playWhiteNoise, std::ref(mixer));
    audio_thread.join();

    return 0;
}
