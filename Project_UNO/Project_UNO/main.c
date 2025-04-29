/*************************************************************
 * 3.  main_uno.c  – slave (ATmega328P)
 *************************************************************/
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include "delay.h"
#include "lcd.h"
#include "protocol.h"

/* --- LED pin definitions (see user wiring) --- */
#define MOV_LED_PORT PORTB
#define MOV_LED_DDR  DDRB
#define MOV_LED_PIN  PB0   /* Arduino D8 */

#define DOOR_LED_PORT PORTB
#define DOOR_LED_DDR  DDRB
#define DOOR_LED_PIN  PB1   /* Arduino D9 */

/* --- Buzzer pin --- */
#define BUZZER_PIN PD7

/* ---------------- SPI slave ---------------- */
static void spi_slave_init(void)
{
    DDRB |= (1<<PB4);      /* MISO output            */
    SPCR = (1<<SPE) | (1<<SPIE);   /* Enable SPI + interrupt */
}

/* ---------------- Timer1: Buzzer melody ---------------- */
static void timer1_init(void)
{
    TCCR1B |= (1 << WGM12); // CTC mode
    TIMSK1 |= (1 << OCIE1A); // Enable compare interrupt
}

ISR(TIMER1_COMPA_vect)
{
    PORTD ^= (1 << BUZZER_PIN); // Toggle buzzer
}

static void play_note(uint16_t ocr1a_val, uint16_t duration_ms)
{
    OCR1A = ocr1a_val;
    TCCR1B |= (1 << CS10); // Start Timer1, no prescaler

    while (duration_ms--) _delay_ms(1);

    TCCR1B &= ~(1 << CS10); // Stop Timer1
    PORTD &= ~(1 << BUZZER_PIN); // Ensure buzzer is off
    _delay_ms(50); // Short pause
}

static void play_melody(void)
{
    play_note(27235, 62);   // D4
    play_note(27235, 62);   // D4
    play_note(13617, 104);  // D5
    play_note(18181, 104);  // A4
    play_note(19230, 250);  // G#4
    play_note(20408, 250);  // G4
    play_note(22907, 62);   // F4
    play_note(27235, 62);   // D4
    play_note(22907, 62);   // F4
    play_note(20408, 62);   // G4
    play_note(15287, 62);   // C5
}



/* ---------------- SPI Command ISR ---------------- */
ISR(SPI_STC_vect)
{
    uint8_t cmd = SPDR;    /* latched byte from master */
    switch(cmd)
    {
    case CMD_MOVEMENT_LED_ON:  MOV_LED_PORT |=  _BV(MOV_LED_PIN); break;
    case CMD_MOVEMENT_LED_OFF: MOV_LED_PORT &= ~_BV(MOV_LED_PIN); break;
    case CMD_DOOR_LED_ON:      DOOR_LED_PORT |=  _BV(DOOR_LED_PIN); break;
    case CMD_DOOR_LED_OFF:     DOOR_LED_PORT &= ~_BV(DOOR_LED_PIN); break;
	
    default: break; /* ignore unknown */
    }
}

/* ---------------- Main ---------------- */
int main(void)
{
    /* LED setup */
    MOV_LED_DDR  |= _BV(MOV_LED_PIN);
    DOOR_LED_DDR |= _BV(DOOR_LED_PIN);
    MOV_LED_PORT &= ~_BV(MOV_LED_PIN);
    DOOR_LED_PORT &= ~_BV(DOOR_LED_PIN);

    /* Buzzer setup */
    DDRD |= (1 << BUZZER_PIN);
    PORTD &= ~(1 << BUZZER_PIN);

    /* Init modules */
    //spi_slave_init();
    //timer1_init();
    //lcd_init(LCD_DISP_ON);
    //lcd_clrscr();
    //show_idle();

    sei(); // Enable global interrupts

    while(1) {
        // All work handled in ISRs
    }
}
