#ifndef I2C_MANAGER_H
#define I2C_MANAGER_H

#include <stdint.h>  // Needed for uint8_t
#include <stddef.h>  // Needed for size_t
#include <fcntl.h>       // Needed for open()
#include <sys/ioctl.h>   // Needed for ioctl()
#include <linux/i2c-dev.h> // Needed for I2C_SLAVE
#include <stdio.h>
#include <unistd.h>      // Needed for close(), read(), write()

int I2C_Open();               // Open the I2C device
int I2C_SetDevice(uint8_t address); // Switch I2C device address
void I2C_Close();             // Close the I2C device
int I2C_Write(uint8_t device_addr, uint8_t mode, uint8_t data); // Write data to I2C
int I2C_Read(uint8_t device_addr, uint8_t* buffer, size_t length); // Read data from I2C

extern int i2c_fd; // Shared I2C file descriptor

#endif // I2C_MANAGER_H

