#ifndef MEGA_LIB_H
#define MEGA_LIB_H

#include <stdint.h>

// Initialize Mega-specific hardware (e.g., emergency button pins)
void mega_init(void);

// Process the floor selection from the keypad
void process_floor_selection(uint8_t floor);

#endif // MEGA_LIB_H
