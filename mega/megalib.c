#include "mega_lib.h"
#include <avr/io.h>
#include <util/delay.h>

void mega_init(void) {
    // Example: Configure PD4 as input for an emergency button with internal pull-up enabled.
    DDRD &= ~(1 << PD4); // Set PD4 as input.
    PORTD |= (1 << PD4); // Enable pull-up resistor.
}

void process_floor_selection(uint8_t floor) {
    // Process the selected floor (stub for further logic)
    // You can expand with additional functionality as required.
    _delay_ms(500);
}
