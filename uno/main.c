#include <avr/io.h>
#include <util/delay.h>
#include "uno_lib.h"  // UNO-specific functions

int main(void) {
    // Initialize UNO peripherals such as LEDs and buzzer
    uno_init();
    
    while(1) {
        // Execute the UNO function to update outputs, for example blinking an LED or triggering the buzzer.
        uno_execute_movement();
    }
    
    return 0;
}
