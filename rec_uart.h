#ifndef REC_UART_H
#define REC_UART_H

#include <stdint.h>
#include <thread>
#include <mutex>

#define UART_PORT "/dev/ttyAMA2"

class RecUart
{
public:
    // Constructor & Destructor
    RecUart();
    ~RecUart();

    // Get heart rate and SpO2 data (thread-safe)
    void getData(uint8_t& bpm, float& spo2);

private:
    // Serial port file descriptor
    int uart_fd;

    // Thread and mutex
    std::thread uart_thread;
    std::mutex data_mutex;

    // Running state
    bool running;

    // Heart rate and SpO2 variables (protected by mutex)
    uint8_t bpm;
    float spo2_percent;

    // Serial port listening thread function
    void listen();
};

#endif // REC_UART_H

