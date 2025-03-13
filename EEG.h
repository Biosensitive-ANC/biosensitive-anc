#ifndef EEG_H
#define EEG_H

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <cstdlib>

#define SERIAL_PORT "/dev/ttyUSB0"

class EEGSerial {
private:
    int fd;                     // File descriptor for the serial port
    uint8_t receive[36];        // Buffer for received data
    bool receive_ok;            // Flag indicating data reception is complete
    std::thread receiveThread;  // Thread for listening to the serial port
    std::mutex dataMutex;       // Mutex to protect shared data

public:
    uint8_t attention;   // attention level
    uint8_t meditation;  // meditation level

    EEGSerial();
    ~EEGSerial();

    void configureSerialPort();
    void receiveData();
    void processEEGData();
    void startListening();  // Start the receiving thread
    void stopListening();   // Stop the receiving thread
    void getData(uint8_t* out_attention, uint8_t* out_meditation);
};

#endif // EEG_H
