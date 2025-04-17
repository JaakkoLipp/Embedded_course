/*
 * main.c  — Arduino Mega (ATmega2560) as SPI master,
 * keypad + LCD on Mega, movement/door LEDs on UNO via SPI.
 */

#define F_CPU 16000000UL
#define BAUD  9600
#define FOSC  16000000UL
#define MYUBRR (FOSC/16/BAUD - 1)

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <stdlib.h>
#include "keypad.h"
#include "lcd.h"

// — UART for printf/scanf —
static void USART_init(uint16_t ubrr) {
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr & 0xFF;
    UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
    UCSR0C |= (1<<USBS0)|(3<<UCSZ00);  // 8 data, 2 stop
}
static void USART_Transmit(unsigned char data, FILE *stream) {
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}
static char USART_Receive(FILE *stream) {
    while (!(UCSR0A & (1<<RXC0)));
    return UDR0;
}
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input  = FDEV_SETUP_STREAM(NULL, USART_Receive,  _FDEV_SETUP_READ);

// — SPI master on Mega2560 —
#define SS_PIN    PB0  // digital 53
#define MOSI_PIN  PB2  // digital 51
#define SCK_PIN   PB1  // digital 52

static void SPI_master_init(void) {
    DDRB |= (1<<SS_PIN)|(1<<MOSI_PIN)|(1<<SCK_PIN);
    SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR0);  // enable, master, fosc/16
}

// Lower SS, send cmd + '\r', raise SS
static void send_command_to_slave(char cmd) {
    PORTB &= ~(1<<SS_PIN);
    SPDR = cmd;
    while (!(SPSR & (1<<SPIF)));
    _delay_us(5);

    SPDR = '\r';
    while (!(SPSR & (1<<SPIF)));
    _delay_us(5);

    PORTB |= (1<<SS_PIN);
}

static inline void blinkLED8_to_slave(void) { send_command_to_slave('8'); }
static inline void blinkLED9_to_slave(void) { send_command_to_slave('9'); }

// Initialize LCD & keypad
static void setup_display_and_keypad(void) {
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_puts("Choose floor");
    _delay_ms(1000);
    KEYPAD_Init();
}

int main(void) {
    setup_display_and_keypad();

    // UART for debugging (optional)
    USART_init(MYUBRR);
    stdout = &uart_output;
    stdin  = &uart_input;

    // SPI
    SPI_master_init();
    SPDR = 0xFF;   // prime SPDR

    while (1) {
        // 1) read floor from keypad (supports 0–9 here; extend for multi?digit)
        uint8_t key = KEYPAD_GetKey();
        _delay_ms(200);
        if (key != 0xFF) {
            // show the floor
            char buf[3] = { (char)key, '\0' };
            lcd_clrscr();
            lcd_puts("Floor: ");
            lcd_puts(buf);

            // 2) start movement ? blink movement LED on UNO
            blinkLED8_to_slave();

            // (You'd normally step floor by floor, updating LCD,
            //  but here we just simulate a 2?s ride.)
            _delay_ms(3000);

            // 3) arrived ? blink door LED on UNO
            blinkLED9_to_slave();

            // show door open on LCD
            lcd_clrscr();
            lcd_puts("Door open");
            _delay_ms(2000);

            // back to idle
            lcd_clrscr();
            lcd_puts("Choose floor");
        }
    }
    return 0;
}
