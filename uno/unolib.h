#ifndef UNO_LIB_H
#define UNO_LIB_H

// Initialize UNO-specific hardware (LEDs, buzzer, etc.)
void uno_init(void);

// Execute elevator movement functions (e.g., updating LEDs, buzzer control)
void uno_execute_movement(void);

#endif // UNO_LIB_H
