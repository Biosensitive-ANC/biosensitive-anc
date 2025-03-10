#include "i2c_manager.h"


// **Actual Definition of `i2c_fd` (Memory is allocated here)**
int i2c_fd = -1;  // Default to -1, meaning I2C is not yet opened

/**
 * @brief Open the I2C device (only once)
 */
int I2C_Open() {
    if (i2c_fd >= 0) return 0; // Already opened

    i2c_fd = open("/dev/i2c-1", O_RDWR);
    if (i2c_fd < 0) {
        perror("I2C open error");
        return -1;
    }
    return 0;
}

/**
 * @brief Set I2C device address
 */
int I2C_SetDevice(uint8_t address) {
    if (i2c_fd < 0) {
        fprintf(stderr, "I2C device not opened\n");
        return -1;
    }
    if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
        perror("I2C ioctl error");
        return -1;
    }
    return 0;
}

/**
 * @brief Close the I2C device
 */
void I2C_Close() {
    if (i2c_fd >= 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
}

/**
 * @brief Write data to an I2C device
 */
int I2C_Write(uint8_t device_addr, uint8_t mode, uint8_t data) {
    if (I2C_SetDevice(device_addr) < 0) return -1;
    uint8_t buffer[2] = { mode, data };
    return (write(i2c_fd, buffer, 2) == 2) ? 0 : -1;
}

/**
 * @brief Read data from an I2C device
 */
int I2C_Read(uint8_t device_addr, uint8_t* buffer, size_t length) {
    if (I2C_SetDevice(device_addr) < 0) return -1;
    return read(i2c_fd, buffer, length);
}
