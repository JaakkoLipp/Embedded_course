/*************************************************************
 * 2.  main_mega.c  – master (ATmega2560)
 *************************************************************/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "delay.h"
#include "lcd.h"
#include "keypad.h"
#include "protocol.h"

/* ---------------- SPI master helpers ---------------- */
static void spi_master_init(void)
{
    /* SCK = PB1, MOSI = PB2, SS = PB0  → outputs; MISO = PB3 input */
    DDRB |= (1<<PB1) | (1<<PB2) | (1<<PB0);
    DDRB &= ~(1<<PB3);
    /* Keep SS high (idle) */
    PORTB |= (1<<PB0);
    /* Enable SPI, Master, f_osc/16 (1 MHz at 16 MHz clock) */
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
}

static uint8_t spi_tx(uint8_t data)
{
    SPDR = data;
    while (!(SPSR & (1<<SPIF)));
    return SPDR;          /* discard slave reply for now */
}

static void spi_cmd(uint8_t cmd)
{
    PORTB &= ~(1<<PB0);   /* SS low  */
    spi_tx(cmd);
    PORTB |=  (1<<PB0);   /* SS high */
}

/* ---------------- small helpers ---------------- */
static uint8_t is_digit(char c){ return (c>='0' && c<='9'); }
static void led_movement_on (void){ spi_cmd(CMD_MOVEMENT_LED_ON); }
static void led_movement_off(void){ spi_cmd(CMD_MOVEMENT_LED_OFF);} 
static void led_door_on      (void){ spi_cmd(CMD_DOOR_LED_ON);    }
static void led_door_off     (void){ spi_cmd(CMD_DOOR_LED_OFF);   }

/* ---------------- emergency helpers ---------------- */
#define EMERGENCY_BUTTON_PIN PD2

static void emergency_button_init(void)
{
    DDRD &= ~(1 << EMERGENCY_BUTTON_PIN);   // Input
    PORTD |=  (1 << EMERGENCY_BUTTON_PIN);  // Pull-up enabled
}

static uint8_t emergency_button_pressed(void)
{
    return !(PIND & (1 << EMERGENCY_BUTTON_PIN)); // Active low
}

static void handle_emergency(void)
{
    lcd_clrscr();
    lcd_puts("EMERGENCY");

    for (uint8_t i = 0; i < 3; i++) {
        led_movement_on();
        DELAY_ms(300);
        led_movement_off();
        DELAY_ms(300);
    }

    led_door_on();
    spi_cmd(CMD_EMERGENCY);  // Command to uno to play melody
    DELAY_ms(1500);
    led_door_off();

    // Wait for button release to avoid repeated triggering
    while (emergency_button_pressed());
    _delay_ms(200);
}

/* ---------------- main state machine ---------------- */
#define FLOOR_TIME_SEC 3      /* 1 s per floor in this simple simulation */

enum state_t { ST_IDLE, ST_MOVING, ST_DOOR };

int main(void)
{
    uint8_t  current_floor = 0;
    uint8_t  target_floor  = 0;
    enum state_t state = ST_IDLE;

    KEYPAD_Init();
    spi_master_init();
    emergency_button_init();

    lcd_init(LCD_DISP_ON);
    lcd_clrscr();

    while (1)
    {
        // --- Check emergency button each loop ---
        if (emergency_button_pressed()) {
            handle_emergency();
            state = ST_IDLE; // return to idle after emergency
            continue;
        }

        switch(state)
        {
        /* ------------------------------------------------ IDLE */
        case ST_IDLE:
            lcd_clrscr();
            lcd_puts("Choose floor:");

            /* Wait first digit */
            char d1;
            do{ d1 = KEYPAD_GetKey(); } while(!is_digit(d1));
            lcd_gotoxy(0,1); lcd_putc(d1);

            /* Wait second digit */
            char d2;
            do{ d2 = KEYPAD_GetKey(); } while(!is_digit(d2));
            lcd_putc(d2);

            target_floor = (d1 - '0')*10 + (d2 - '0');

            if(target_floor == current_floor)
            {
                /* Fault: blink movement LED 3× and stay in IDLE */
                for(uint8_t i=0;i<3;i++){ led_movement_on(); DELAY_ms(500); led_movement_off(); DELAY_ms(500);}                
            }
            else
            {
                state = ST_MOVING;
            }
            break;

        /* ------------------------------------------------ MOVING */
        case ST_MOVING:
            led_movement_on();
            int8_t dir = (target_floor > current_floor) ? 1 : -1;
            while(current_floor != target_floor)
            {
                current_floor += dir;
                /* update LCD second line with current floor */
                lcd_gotoxy(0,1);
                char buf[17];
                itoa(current_floor, buf, 10);
                lcd_puts("Floor ");
                lcd_puts(buf);
                /* simple time model */
                DELAY_sec(FLOOR_TIME_SEC);
            }
            led_movement_off();
            state = ST_DOOR;
            break;

        /* ------------------------------------------------ DOOR OPEN/CLOSE */
        case ST_DOOR:
            led_door_on();
            lcd_clrscr();
            lcd_puts("Door opening...");
            DELAY_sec(5);
            led_door_off();
            lcd_clrscr();
            lcd_puts("Door closed");
            DELAY_sec(1);
            state = ST_IDLE;
            break;
        }
    }
}
