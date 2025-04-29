/***************************************************************
 * Project  : Elevator Simulator (BL40A1812 course)
 * File     : main_uno.c   ─ ATmega328P  (Arduino-UNO, SPI-slave)
 * Purpose  : Drive two status LEDs and a piezo buzzer according
 *            to single-byte commands sent by the MEGA master.
 * Author   : <your-names>
 * Licence  : MIT
 ****************************************************************/

 #define F_CPU 16000000UL
 #include <avr/io.h>
 #include <avr/interrupt.h>
 #include <util/delay.h>
 #include "delay.h"
 #include "protocol.h"
 
 /*--------------------------------------------------------------------
   GPIO pin map (Arduino UNO)
   ------------------------------------------------------------------*/
 #define MOV_LED_PIN   PB0              /* D8  – movement indicator   */
 #define DOOR_LED_PIN  PB1              /* D9  – door indicator       */
 #define BUZZER_PIN    PD3              /* D3  – piezo driven by T1   */
 
 /*--------------------------------------------------------------------
   Runtime flags set from ISR context
   ------------------------------------------------------------------*/
 volatile uint8_t buzzer_request = 0;   ///< 1 → play full emergency melody once
 volatile uint8_t ding_request   = 0;   ///< 1 → play short arrival chime
 
 /*====================================================================
   1.  SPI-slave setup  (8 bit commands from MEGA master)
   ====================================================================*/
 static void spi_slave_init(void)
 {
     DDRB |= _BV(PB4);                  /* MISO as output               */
     SPCR  = _BV(SPE) | _BV(SPIE);      /* enable SPI + interrupt       */
 }
 
 /** SPI transfer-complete ISR  
  *  Decodes one-byte opcode and sets flags or toggles LEDs instantly.
  */
 ISR(SPI_STC_vect)
 {
     switch (SPDR)
     {
         case CMD_MOVEMENT_LED_ON:   PORTB |=  _BV(MOV_LED_PIN);  break;
         case CMD_MOVEMENT_LED_OFF:  PORTB &= ~_BV(MOV_LED_PIN);  break;
         case CMD_DOOR_LED_ON:       PORTB |=  _BV(DOOR_LED_PIN); break;
         case CMD_DOOR_LED_OFF:      PORTB &= ~_BV(DOOR_LED_PIN); break;
         case CMD_BUZZER_PLAY_ONESHOT: buzzer_request = 1;        break;
         case CMD_DING:                ding_request   = 1;        break;
         default: break; /* unknown opcodes are ignored            */
     }
 }
 
 /*====================================================================
   2.  Buzzer driver  (Timer-1 CTC toggles PD3 at note frequency)
   ====================================================================*/
 static void buzzer_init(void)
 {
     DDRD  |= _BV(BUZZER_PIN);          /* PD3 → output                */
     TCCR1A = 0;                        /* normal port operation       */
     TCCR1B = _BV(WGM12);               /* CTC mode, clock stopped     */
 }
 
 /* Toggle pin every compare-match (50 % duty-cycle square wave) */
 ISR(TIMER1_COMPA_vect)
 {
     PORTD ^= _BV(BUZZER_PIN);
 }
 
 /* ---------- low-level helpers ---------- */
 static void tone_start(uint16_t ocr1a_val)
 {
     OCR1A  = ocr1a_val;
     TCNT1  = 0;
     TCCR1B |= _BV(CS10);               /* clk / 1                     */
     TIMSK1 |= _BV(OCIE1A);             /* enable toggle ISR           */
 }
 
 static void tone_stop(void)
 {
     TCCR1B &= ~_BV(CS10);              /* stop clock                  */
     TIMSK1 &= ~_BV(OCIE1A);            /* disable ISR                 */
     PORTD  &= ~_BV(BUZZER_PIN);        /* ensure low level            */
 }
 
 /* ---------- high-level helpers ---------- */
 /** Play a single note for *ms* milliseconds. */
 static void play_note(uint16_t ocr, uint16_t ms)
 {
     tone_start(ocr);
     while (ms--) _delay_ms(1);
     tone_stop();
     _delay_ms(50);                     /* inter-note gap              */
 }
 
 /** Full 11-note emergency melody (≈1.7 s). */
 static void play_melody(void)
 {
     play_note(27235, 62); play_note(27235, 62);  /* D4 D4 */
     play_note(13617,104); play_note(18181,104);  /* D5 A4 */
     play_note(19230,250); play_note(20408,250);  /* G#4 G4*/
     play_note(22907, 62); play_note(27235, 62);  /* F4 D4 */
     play_note(22907, 62); play_note(20408, 62);  /* F4 G4 */
     play_note(15287, 62);                         /* C5   */
 }
 
 /** One-shot arrival chime (≈500 Hz, 100 ms). */
 static inline void play_ding(void)
 {
     play_note(16000, 100);             /* OCR1A=16000 → ~500 Hz       */
 }
 
 /*====================================================================
   3.  Application entry point
   ====================================================================*/
 int main(void)
 {
     /* LED pins → output, start OFF */
     DDRB  |= _BV(MOV_LED_PIN) | _BV(DOOR_LED_PIN);
     PORTB &= ~(_BV(MOV_LED_PIN) | _BV(DOOR_LED_PIN));
 
     buzzer_init();
     spi_slave_init();
     sei();                             /* global IRQ enable           */
 
     /* ---------- super-loop ---------- */
     for (;;)
     {
         if (buzzer_request) {          /* play full melody once       */
             buzzer_request = 0;
             play_melody();
         }
 
         if (ding_request) {            /* play single arrival chime   */
             ding_request = 0;
             play_ding();
         }
         /* CPU sleeps in idle between ISR events (optional)          */
     }
 }
 