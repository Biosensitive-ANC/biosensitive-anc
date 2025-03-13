#include "rec_uart.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// Constructor
RecUart::RecUart() : uart_fd(-1), running(false), bpm(0), spo2_percent(0.0f)
{
    // Open the serial port
    uart_fd = open(UART_PORT, O_RDWR | O_NOCTTY);
    if (uart_fd == -1)
    {
        std::cerr << "Error opening UART: " << UART_PORT << std::endl;
        return;
    }

    // Configure serial port parameters
    struct termios options;
    tcgetattr(uart_fd, &options);

    // Set baud rate to 9600
    cfsetispeed(&options, B9600);
    cfsetospeed(&options, B9600);

    // Configure serial port parameters (8N1: 8 data bits, no parity, 1 stop bit)
    options.c_cflag = CS8 | CLOCAL | CREAD; // Set baud rate and 8N1 mode
    options.c_iflag = IGNPAR; // Ignore parity errors
    options.c_oflag = 0;      // Raw output
    options.c_lflag = 0;      // Raw input

    tcflush(uart_fd, TCIFLUSH); // Flush the input buffer
    tcsetattr(uart_fd, TCSANOW, &options); // Apply settings immediately

    // Start the listening thread
    running = true;
    uart_thread = std::thread(&RecUart::listen, this);
}

// Listen to serial port data
void RecUart::listen()
{
    uint8_t buffer[2];

    while (running)
    {
        int bytes_received = 0;
        while (bytes_received < 2)
        {
            int bytes_read = read(uart_fd, buffer + bytes_received, 2 - bytes_received);
            if (bytes_read > 0)
            {
                bytes_received += bytes_read;
            }
        }

        // Update data (with mutex lock)

        // std::lock_guard<std::mutex> lock(data_mutex);
        bpm = buffer[0];
        spo2_percent = 90.0f + (buffer[1] * 10.0f / 255.0f);
    }

    std::cout << "REC ENDED" << std::endl;
}

// Get heart rate and SpO2 (thread-safe)
void RecUart::getData(uint8_t* out_bpm, float* out_spo2)
{
    // std::lock_guard<std::mutex> lock(data_mutex);
    *out_bpm = bpm;
    *out_spo2 = spo2_percent;
}

// Destructor (close serial port & thread)
RecUart::~RecUart()
{
    running = false;
    if (uart_thread.joinable())
    {
        uart_thread.join();
    }
    if (uart_fd != -1)
    {
        close(uart_fd);
        uart_fd = -1;
    }
}
