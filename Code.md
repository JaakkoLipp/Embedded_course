# Code-Level Walk-Through

_(“How does this thing actually work?”)_

This document complements the project’s high-level README by drilling down into the **code architecture, key algorithms, and interrupt timing** on both AVR boards. If you need to tweak the firmware or port it to a different MCU, start here.

---

## 1 Repository map

```
Project_MEGA/            →  ATmega2560 (master)
│   main.c               –  high-level FSM + drivers
│   lcd.c / lcd.h        –  course LCD library (unchanged)
│   keypad.c / keypad.h  –  4×4 keypad driver (unchanged)
│   protocol.h           –  shared 1-byte opcode list
│   ...
Project_UNO/             →  ATmega328P (slave)
    main.c               –  LED + buzzer drivers, SPI-ISR
docs/                    →  schematic, state-diagram, demo GIF
```

---

## 2 MEGA (main.c) breakdown

### 2.1 Global IRQs

| Vector                | Purpose                            | Runs                                                             |
| --------------------- | ---------------------------------- | ---------------------------------------------------------------- |
| **INT4_vect**         | Emergency push button (active-low) | As soon as the line falls → sets `emg_flag = 1`                  |
| **TIMER1_COMPA_vect** | **100 Hz system tick**             | Increments `tick10ms`; every software delay is derived from this |

### 2.2 State machine (FSM)

```text
+--------+    floor ≠ cur        +-----------+
|  IDLE  | ────────────────►     |  MOVING   |
+--------+         ▲             +-----------+
   ▲  ▲            | emg_flag                   floor reached
   |  | cur=floor   +-----------+               emg_flag
   |  |                          |              +--------+
   |  +--------------------------+              |EMERGENCY|
   |      blink 3×                               +--------+
   |                                             |  Door  |
   +---------------------------------------------+--------+
```

_Implementation:_ `switch(state)` in the main `while(1)` loop.  
Each state schedules waits via `wait_ms(x)` (non-blocking) so external interrupts stay live.

### 2.3 SPI helper

```c
static void spi_cmd(uint8_t cmd)
{
    PORTB &= ~_BV(PB0);   // SS low
    SPDR = cmd;           // start transfer
    while (!(SPSR & _BV(SPIF)));
    PORTB |=  _BV(PB0);   // SS high (latch)
}
```

All higher-level helpers (`led_movement_on()`, `spi_cmd(CMD_DING)` …) collapse to a single `spi_cmd()` call, so adding new opcodes is trivial.

---

## 3 UNO (main.c) breakdown

### 3.1 SPI receive ISR

```c
ISR(SPI_STC_vect)
{
    switch (SPDR) {
      case CMD_MOVEMENT_LED_ON:   LED_ON(MOV); break;
      case CMD_MOVEMENT_LED_OFF:  LED_OFF(MOV); break;
      case CMD_DOOR_LED_ON:       LED_ON(DOOR); break;
      case CMD_DOOR_LED_OFF:      LED_OFF(DOOR); break;
      case CMD_BUZZER_PLAY_ONESHOT: buzzer_request = 1; break;
      case CMD_DING:                ding_request   = 1; break;
    }
}
```

### 3.2 Tone generation (Timer-1 toggle)

```c
ISR(TIMER1_COMPA_vect) { PORTD ^= _BV(BUZZER_PIN); }

static void tone_start(uint16_t ocr)
{
    OCR1A  = ocr;
    TCNT1  = 0;
    TCCR1B |= _BV(CS10);           // clk/1
    TIMSK1 |= _BV(OCIE1A);
}
```

`play_note()` and `play_ding()` wrap `tone_start/stop` to create either

- an 11-note emergency melody (≈1.7 s), or
- a single 100 ms “ding”.

Main loop simply checks two flags and calls the appropriate player; nothing else blocks.

---

## 4 Timing & constants

| Symbol           | Board | Value       | Meaning                      |
| ---------------- | ----- | ----------- | ---------------------------- |
| `F_CPU`          | both  | 16 000 000  | core clock                   |
| `TIMER1_COMPA`   | MEGA  | 10 ms       | system tick (100 Hz)         |
| `FLOOR_TIME_SEC` | MEGA  | 250 ms      | simulated travel per floor   |
| `OCR1A` values   | UNO   | 27235…13617 | Pre-computed for D4, D5, A4… |

---

## 5 Extending the protocol

Add a new command in `protocol.h`, for example:

```c
#define CMD_OVERLOAD_FAULT 0x30
```

- MEGA side → call `spi_cmd(CMD_OVERLOAD_FAULT);`
- UNO side → handle in `SPI_STC_vect` (blink both LEDs, sound alarm, etc.)

Because each command is a **single byte** and the link is fire-and-forget, the bandwidth overhead is negligible (one byte ≈ 8 µs @1 MHz).

---

## 6 Porting notes

- **Clock** – if you migrate to a 20 MHz part, adjust `F_CPU` and the `OCR1A` tone table.
- **I²C instead of SPI** – swap `spi_cmd()` with `twi_write_byte()`, the rest of the logic is identical.
- **Bare AVR (no Arduino)** – LCD and keypad libraries rely only on `<avr/io.h>`; remove the Arduino core and keep the same pin mapping.

---

## 7 Troubleshooting checklist

| Symptom                  | Likely cause                                                                                        |
| ------------------------ | --------------------------------------------------------------------------------------------------- |
| Buzzer silent            | • `BUZZER_PIN` wired to wrong UNO pin.<br>• Timer-1 clock not started (`TCCR1B & CS10`).            |
| Emergency button ignored | • Wired to wrong MEGA header pin (must be **D2/PE4**).<br>• `EIMSK` or `EICRB` mis-set.             |
| SPI not working          | • SS line left floating (hold PB0 high on MEGA when idle).<br>• UNO PB2 accidentally set to output. |

---

## 8 Credits

_Planning and Implementation_ – Group 28
_Documentation and general consultation help_ – ChatGPT-o3
_Course and peripheral libraries_ – LUT & Lab staff
