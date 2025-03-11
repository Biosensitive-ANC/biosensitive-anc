#include <iostream>
#include <thread>
#include "ancMixing.h"
#include "whiteNoise.h"
#include "playback.h"

int main() {
    std::cout << "Playing adjustable white noise..." << std::endl;

    Gains gains;
    AncMixing mixer(gains);

    playWhiteNoise(mixer);

    return 0;
}
