/*************************************************************
 * 3.  main_uno.c  – slave (ATmega328P)
 *************************************************************/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "delay.h"
#include "protocol.h"

/* --- LED pin definitions (see user wiring) --- */
#define MOV_LED_PORT PORTB
#define MOV_LED_DDR  DDRB
#define MOV_LED_PIN  PB0   /* Arduino D8 */

#define DOOR_LED_PORT PORTB
#define DOOR_LED_DDR  DDRB
#define DOOR_LED_PIN  PB1   /* Arduino D9 */

/* ---------------- SPI slave ---------------- */
static void spi_slave_init(void)
{
    DDRB |= (1<<PB4);      /* MISO output            */
    SPCR = (1<<SPE) | (1<<SPIE);   /* Enable SPI + interrupt */
}

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

int main(void)
{
    /* Configure LED pins as output */
    MOV_LED_DDR  |= _BV(MOV_LED_PIN);
    DOOR_LED_DDR |= _BV(DOOR_LED_PIN);
    /* Initial off */
    MOV_LED_PORT &= ~_BV(MOV_LED_PIN);
    DOOR_LED_PORT &= ~_BV(DOOR_LED_PIN);

    spi_slave_init();
    sei();

    while(1){ /* all work done in ISR */ }
}
