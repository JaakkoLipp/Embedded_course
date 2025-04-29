/*************************************************************
 * 2.  main_mega.c  � master (ATmega2560)
 *************************************************************/

#define F_CPU 16000000UL
#define EMG_PIN   PE4
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "delay.h"
#include "lcd.h"
#include "keypad.h"
#include "protocol.h"

volatile uint8_t emg_flag = 0;

ISR(INT4_vect) { emg_flag = 1; }

/* ---------------- SPI master helpers ---------------- */
static void spi_master_init(void)
{
    /* SCK�=�PB1, MOSI�=�PB2, SS�=�PB0  ? outputs; MISO�PB3 input */
    DDRB |= (1<<PB1) | (1<<PB2) | (1<<PB0);
    DDRB &= ~(1<<PB3);
    /* Keep SS high (idle) */
    PORTB |= (1<<PB0);
    /* Enable SPI, Master, f_osc/16 (1�MHz at 16�MHz clock) */
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

/* ---------------- main state machine ---------------- */
#define FLOOR_TIME_SEC 250

enum state_t { ST_IDLE, ST_MOVING, ST_DOOR, ST_EMERGENCY };
    
int main(void)
{
    /* --- emergency button init --- */
    DDRE  &= ~_BV(EMG_PIN);      /* make PE4 input            */
    PORTE |=  _BV(EMG_PIN);      /* enable internal pull-up   */

    EICRB |=  _BV(ISC41);        /* INT4: falling edge        */
    EIMSK |=  _BV(INT4);         /* enable INT4               */
    sei();                       /* << enable global IRQs     */
    
    // operation
    uint8_t  current_floor = 0;
    uint8_t  target_floor  = 0;
    enum state_t state = ST_IDLE;

    KEYPAD_Init();
    spi_master_init();

    lcd_init(LCD_DISP_ON);
    lcd_clrscr();

    while (1)
    {
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
                /* Fault: blink movement LED 3� and stay in IDLE */
                for(uint8_t i=0;i<3;i++){ led_movement_on(); _delay_ms(500); led_movement_off(); _delay_ms(500);}
            }
            else
            {
                state = ST_MOVING;
            }
            break;

        /* ------------------------------------------------ MOVING */
        case ST_MOVING:
            led_movement_on();
            {
                int8_t dir = (target_floor > current_floor) ? 1 : -1;

                /* move floor-by-floor until target OR emergency */
                while (current_floor != target_floor && !emg_flag)
                {
                    current_floor += dir;

                    /* update LCD second line */
                    lcd_gotoxy(0,1);
                    char line[17];
                    sprintf(line, "Floor %02u", current_floor); /* always 00‥99 */
                    lcd_puts(line);
                    
                    _delay_ms(FLOOR_TIME_SEC);   /* still interrupt-friendly */
                }
            }
            led_movement_off();

            /* decide next state */
            if (emg_flag)
            state = ST_EMERGENCY;
            else
            state = ST_DOOR;
            break;


        /* ------------------------------------------------ DOOR OPEN/CLOSE */
        case ST_DOOR:
            led_door_on();
            lcd_clrscr();
            lcd_puts("Door opening...");
            _delay_ms(5000);
            led_door_off();
            lcd_clrscr();
            lcd_puts("Door closed");
            _delay_ms(1500);
            state = ST_IDLE;
            break;
            
    /* ------------------------------------------------ EMERGENCY  (minimum level) */
    case ST_EMERGENCY:
        emg_flag = 0;
        lcd_clrscr();
        lcd_puts("!!! EMERGENCY !!!");

        /* 1. movement LED blinks 3× */
        for (uint8_t i = 0; i < 3; i++) {
            led_movement_on();  _delay_ms(300);
            led_movement_off(); _delay_ms(300);
        }

        /* 2. open door + buzzer melody (one loop) */
        led_door_on();
        spi_cmd(CMD_BUZZER_PLAY_ONESHOT);
        lcd_clrscr(); lcd_puts("Door opening...");
        _delay_ms(5000);              /* keep door open 5 s */

        /* 3. close door and go idle */
        led_door_off();
        lcd_clrscr(); lcd_puts("Door closed");
        _delay_ms(1500);
        state = ST_IDLE;
        break;

        }
    }
}