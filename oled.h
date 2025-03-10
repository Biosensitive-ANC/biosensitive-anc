#ifndef __OLED_H
#define __OLED_H

#include <stdint.h>
#include <stdio.h>
#include <unistd.h> // usleep()
#include "i2c_manager.h"

// OLED I2C address (default SSD1306 I2C address)
#define OLED_ADDRESS 0x3C

// OLED modes
#define OLED_CMD  0x00  // Command mode
#define OLED_DATA 0x40  // Data mode

// OLED related functions
void OLED_Init(void);       // Initialize OLED
void OLED_Clear(void);      // Clear OLED display
void OLED_Fill(void);       // Turn on all pixels
void OLED_ShowChar(uint8_t x, uint8_t y, char ch);
void OLED_ShowString(uint8_t x, uint8_t y, const char* str);
void OLED_ShowUInt8_twochar(uint8_t x, uint8_t y, uint8_t num);
void OLED_ShowUInt8_threechar(uint8_t x, uint8_t y, uint8_t num);
void OLED_ShowFloat(uint8_t x, uint8_t y, float num);

extern const uint8_t Font8x8[96][8]; // 8x8 font table

#endif // __OLED_H

