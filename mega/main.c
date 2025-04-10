#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"       // Your pre-made LCD library
#include "keypad.h"    // Your pre-made keypad library
#include "mega_lib.h"  // Mega board–specific functions

int main(void) {
    // Initialize Mega board peripherals
    mega_init();
    
    // Initialize LCD and Keypad using already available libraries
    lcd_init();
    keypad_init();

    while(1) {
        // Read floor number input from the keypad
        uint8_t floor = keypad_get_value();
        
        // Display a message on the LCD
        lcd_clear();
        lcd_print("Floor Selected: ");
        lcd_print_number(floor);  // Assume a helper exists to print numbers
        
        // Process the floor selection in a board-specific way if needed
        process_floor_selection(floor);

        _delay_ms(1000);
    }
    
    return 0;
}
