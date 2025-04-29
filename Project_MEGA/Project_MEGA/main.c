/***********************************************************************
 * Project  : Elevator Simulator  (BL40A1812)
 * File     : main_mega.c   — ATmega2560  (master / controller)
 * Purpose  : Read keypad & emergency button, drive LCD, and send
 *            one-byte SPI commands to the UNO slave.
 * Licence  : MIT
 ***********************************************************************/

 #define F_CPU 16000000UL
 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include "lcd.h"
 #include "keypad.h"
 #include "protocol.h"
 
 /*----------------------------------------------------------------------
   GPIO aliases (MEGA)
   --------------------------------------------------------------------*/
 #define EMG_PIN   PE4          /* D2 — emergency button, active-LOW */
 
 /*----------------------------------------------------------------------
   0. Globals & interrupt service routines
   --------------------------------------------------------------------*/
 volatile uint8_t  emg_flag  = 0;    ///< set in INT4 ISR when button pressed
 volatile uint32_t tick10ms  = 0;    ///< 100 Hz system tick (Timer-1 CTC)
 
 /* --- external interrupt: emergency push-button -------------------- */
 ISR(INT4_vect) { emg_flag = 1; }
 
 /* --- time-base interrupt: 10 ms tick ------------------------------ */
 ISR(TIMER1_COMPA_vect) { tick10ms++; }
 
 /* Busy-wait helper that keeps global interrupts enabled */
 static void wait_ms(uint16_t ms)
 {
     const uint32_t target = tick10ms + ms / 10;
     while (tick10ms < target) ;      /* low-power sleep could go here */
 }
 
 /*----------------------------------------------------------------------
   1.  SPI master helpers  (3-wire, 1 MHz)
   --------------------------------------------------------------------*/
 static void spi_master_init(void)
 {
     DDRB |= _BV(PB0) | _BV(PB1) | _BV(PB2);   /* SS, SCK, MOSI outputs */
     DDRB &= ~_BV(PB3);                        /* MISO input            */
     PORTB |= _BV(PB0);                        /* keep SS high (idle)   */
     SPCR  =  _BV(SPE) | _BV(MSTR) | _BV(SPR0);/* enable, clk/16        */
 }
 
 static inline uint8_t spi_tx(uint8_t data)
 {
     SPDR = data;
     while (!(SPSR & _BV(SPIF)));
     return SPDR;                              /* reply not used        */
 }
 
 static inline void spi_cmd(uint8_t cmd)
 {
     PORTB &= ~_BV(PB0);                       /* SS low                */
     spi_tx(cmd);
     PORTB |=  _BV(PB0);                       /* SS high               */
 }
 
 /* One-line wrappers for readability */
 static inline void led_movement_on (void){ spi_cmd(CMD_MOVEMENT_LED_ON); }
 static inline void led_movement_off(void){ spi_cmd(CMD_MOVEMENT_LED_OFF);}
 static inline void led_door_on      (void){ spi_cmd(CMD_DOOR_LED_ON);    }
 static inline void led_door_off     (void){ spi_cmd(CMD_DOOR_LED_OFF);   }
 
 /*----------------------------------------------------------------------
   2.  Finite-state machine defines
   --------------------------------------------------------------------*/
 #define FLOOR_TIME_MS  250                   /* sim-travel per floor  */
 
 typedef enum { ST_IDLE, ST_MOVING,
                ST_DOOR, ST_EMERGENCY } state_t;
 
 /*----------------------------------------------------------------------
   3.  main()
   --------------------------------------------------------------------*/
 int main(void)
 {
     /* --- emergency button input ----------------------------------- */
     DDRE  &= ~_BV(EMG_PIN);
     PORTE |=  _BV(EMG_PIN);                  /* internal pull-up       */
     EICRB |=  _BV(ISC41);                    /* falling edge           */
     EIMSK |=  _BV(INT4);
     sei();
 
     /* --- peripherals ---------------------------------------------- */
     KEYPAD_Init();
     lcd_init(LCD_DISP_ON);
     spi_master_init();
 
     /* --- 10 ms system tick ---------------------------------------- */
     TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10);    /* CTC, /1024 clk */
     OCR1A  = (F_CPU/1024/100) - 1;                  /* 10 ms period   */
     TIMSK1 = _BV(OCIE1A);
 
     /* --- state machine vars --------------------------------------- */
     uint8_t  current_floor = 0;
     uint8_t  target_floor  = 0;
     state_t  state         = ST_IDLE;
 
     /* ===================== super-loop ============================= */
     for (;;)
     {
         switch (state)
         {
         /*-------------------------------------------------- IDLE ----*/
         case ST_IDLE: {
             lcd_clrscr();
             lcd_puts("Choose floor:");
 
             char d1, d2;
             do { d1 = KEYPAD_GetKey(); } while (d1 < '0' || d1 > '9');
             lcd_gotoxy(0,1); lcd_putc(d1);
 
             do { d2 = KEYPAD_GetKey(); } while (d2 < '0' || d2 > '9');
             lcd_putc(d2);
 
             target_floor = (d1-'0')*10 + (d2-'0');
 
             if (target_floor == current_floor) {          /* FAULT */
                 for (uint8_t i=0;i<3;i++){
                     led_movement_on();  wait_ms(500);
                     led_movement_off(); wait_ms(500);
                 }
             } else {
                 state = ST_MOVING;
             }
         } break;
 
         /*------------------------------------------------ MOVING ----*/
         case ST_MOVING: {
             led_movement_on();
             const int8_t dir = (target_floor > current_floor) ?  1 : -1;
 
             while (current_floor != target_floor && !emg_flag)
             {
                 current_floor += dir;
                 lcd_gotoxy(0,1);
                 char buf[17];
                 sprintf(buf,"Floor %02u", current_floor);
                 lcd_puts(buf);
 
                 spi_cmd(CMD_DING);          /* arrival chime          */
                 wait_ms(FLOOR_TIME_MS);
             }
             led_movement_off();
             state = emg_flag ? ST_EMERGENCY : ST_DOOR;
         } break;
 
         /*------------------------------------------------- DOOR -----*/
         case ST_DOOR:
             led_door_on();
             lcd_clrscr(); lcd_puts("Door opening...");
             wait_ms(5000);
             led_door_off();
             lcd_clrscr(); lcd_puts("Door closed");
             wait_ms(1500);
             state = ST_IDLE;
             break;
 
         /*-------------------------------------------- EMERGENCY -----*/
         case ST_EMERGENCY: {
             emg_flag = 0;                       /* clear latch        */
             lcd_clrscr(); lcd_puts("!!! EMERGENCY !!!");
 
             /* one-shot melody  + 3 blinks */
             spi_cmd(CMD_BUZZER_PLAY_ONESHOT);
             for (uint8_t i=0;i<3;i++){
                 led_movement_on();  wait_ms(300);
                 led_movement_off(); wait_ms(300);
             }
 
             lcd_clrscr(); lcd_puts("Press # to open");
             char key;
             do { key = KEYPAD_GetKey(); } while (key != '#');
 
             led_door_on();
             spi_cmd(CMD_DING);                  /* door chime         */
             lcd_clrscr(); lcd_puts("Door opening...");
             wait_ms(5000);
 
             led_door_off();
             lcd_clrscr(); lcd_puts("Door closed");
             wait_ms(1500);
             state = ST_IDLE;
         } break;
         }
     }
 }
 