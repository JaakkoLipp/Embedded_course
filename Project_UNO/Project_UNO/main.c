/*
 * UNO_SPI_LED_SLAVE.c
 *
 * ATmega328P (Arduino UNO) as SPI slave.
 * Receives a single ASCII command byte plus '\r' from the Mega:
 *   '8' → blink movement LED on D8 (PB0)
 *   '9' → blink door LED     on D9 (PB1)
 */

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

// LED pins on PORTB
#define MOV_LED_PIN  PB0    // Arduino D8
#define DOOR_LED_PIN PB1    // Arduino D9

// Command storage
volatile char  spi_cmd = 0;
volatile bool  spi_ready = false;

// Initialize PB0 and PB1 as outputs for the LEDs
static void init_leds(void) {
    DDRB |= (1 << MOV_LED_PIN) | (1 << DOOR_LED_PIN);
}

// Initialize SPI in slave mode, enable interrupt, set MISO (PB4) as output
static void SPI_slave_init(void) {
    DDRB |= (1 << PB4);            // MISO output
    SPCR |= (1 << SPE)  // SPI Enable
          | (1 << SPIE); // SPI Interrupt Enable
    SPDR = 0xFF;                   // prime SPDR
}

// ISR: called on each received SPI byte
ISR(SPI_STC_vect) {
    uint8_t c = SPDR;   // read received byte
    SPDR = 0xFF;        // prepare dummy for next transfer

    if (c == '\r') {
        // end‑of‑command marker: ready to act
        spi_ready = true;
    } else {
        // store latest command byte
        spi_cmd = (char)c;
    }
}

// Blink movement LED (D8) once: on 1 s, off 1 s
static void blink_movement_led(void) {
    PORTB |=  (1 << MOV_LED_PIN);
    _delay_ms(1000);
    PORTB &= ~(1 << MOV_LED_PIN);
    _delay_ms(1000);
}

// Blink door LED (D9) once: on 1 s, off 1 s
static void blink_door_led(void) {
    PORTB |=  (1 << DOOR_LED_PIN);
    _delay_ms(1000);
    PORTB &= ~(1 << DOOR_LED_PIN);
    _delay_ms(1000);
}

int main(void) {
    init_leds();
    SPI_slave_init();
    sei();  // enable global interrupts

    while (1) {
        if (spi_ready) {
            // Execute the command stored in spi_cmd
            switch (spi_cmd) {
                case '8':
                    blink_movement_led();
                    break;
                case '9':
                    blink_door_led();
                    break;
                default:
                    // unknown command: ignore
                    break;
            }
            // reset for next command
            spi_ready = false;
        }
        // idle here until next SPI command
    }
    return 0;
}
