/*************************************************************
 * 3.  main_uno.c  – slave (ATmega328P)
 *     Elevator project: LED driver + one-shot buzzer melody
 *************************************************************/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "delay.h"
#include "protocol.h"

/* ---------------- hardware pins ---------------- */
#define MOV_LED_PIN   PB0          /* D8  */
#define DOOR_LED_PIN  PB1          /* D9  */
#define BUZZER_PIN    PD3          /* D3  – we drive it in an ISR */

/* ---------------- globals ---------------- */
volatile uint8_t buzzer_request = 0;

/* ===========================================================
   SPI  (slave, interrupt-driven)
   ===========================================================*/
static void spi_slave_init(void)
{
    DDRB |= _BV(PB4);             /* MISO out          */
    SPCR  = _BV(SPE) | _BV(SPIE); /* enable + ISR      */
}

ISR(SPI_STC_vect)
{
    uint8_t cmd = SPDR;

    switch (cmd)
    {
    case CMD_MOVEMENT_LED_ON:      PORTB |=  _BV(MOV_LED_PIN);  break;
    case CMD_MOVEMENT_LED_OFF:     PORTB &= ~_BV(MOV_LED_PIN);  break;
    case CMD_DOOR_LED_ON:          PORTB |=  _BV(DOOR_LED_PIN); break;
    case CMD_DOOR_LED_OFF:         PORTB &= ~_BV(DOOR_LED_PIN); break;
    case CMD_BUZZER_PLAY_ONESHOT:  buzzer_request = 1;           break;
    default: break;
    }
}

/* ===========================================================
   BUZZER  (Timer-1 CTC toggling BUZZER_PIN in ISR)
   ===========================================================*/
static void buzzer_init(void)
{
    DDRD  |= _BV(BUZZER_PIN);      /* make PD3 an output */
    TCCR1A = 0;                    /* normal port ops    */
    TCCR1B = _BV(WGM12);           /* CTC, clk stopped   */
}

ISR(TIMER1_COMPA_vect)
{
    PORTD ^= _BV(BUZZER_PIN);      /* toggle every compare match */
}

/* -------- note player helpers -------- */
static void tone_start(uint16_t ocr1a_val)
{
    OCR1A  = ocr1a_val;
    TCNT1  = 0;
    TCCR1B |= _BV(CS10);           /* start Timer1, 1:1 prescale */
    TIMSK1 |= _BV(OCIE1A);         /* enable compare ISR         */
}

static void tone_stop(void)
{
    TCCR1B &= ~_BV(CS10);          /* stop clock        */
    TIMSK1 &= ~_BV(OCIE1A);        /* disable ISR       */
    PORTD  &= ~_BV(BUZZER_PIN);    /* ensure low level  */
}

/* -------- data taken from your melody table -------- */
static void play_note(uint16_t ocr, uint16_t ms)
{
    tone_start(ocr);
    while (ms--) _delay_ms(1);
    tone_stop();
    _delay_ms(50);                 /* short pause */
}

static void play_melody(void)
{
    play_note(27235, 62);   play_note(27235, 62);   /* D4 D4 */
    play_note(13617, 104);  play_note(18181, 104);  /* D5 A4 */
    play_note(19230, 250);  play_note(20408, 250);  /* G#4 G4 */
    play_note(22907, 62);   play_note(27235, 62);   /* F4 D4 */
    play_note(22907, 62);   play_note(20408, 62);   /* F4 G4 */
    play_note(15287, 62);                           /* C5    */
}

/* ===========================================================
   MAIN
   ===========================================================*/
int main(void)
{
    /* LEDs */
    DDRB  |= _BV(MOV_LED_PIN) | _BV(DOOR_LED_PIN);
    PORTB &= ~(_BV(MOV_LED_PIN) | _BV(DOOR_LED_PIN));

    buzzer_init();
    spi_slave_init();
    sei();

    for (;;)
    {
        if (buzzer_request)
        {
            buzzer_request = 0;
            play_melody();         /* ~1.7 s blocking OK on slave */
        }
    }
}
