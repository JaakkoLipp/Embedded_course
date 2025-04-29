# Elevator Project – Presenter’s Demo Guide (updated 2025-05-XX)

---

## 1 Hardware snapshot

| Board                  | Peripheral (pin → signal)                                                                                                                       | Remarks                                           |
| ---------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------- |
| **MEGA 2560 (master)** | Keypad ROW/COL → PORTK (A8-A15) <br>LCD 4-bit → PORTA (D22-D29) <br>SPI PB0/1/2/3 (SS/SCK/MOSI/MISO) <br>Emergency button → **D2 (PE4 → INT4)** | PE4 uses internal pull-up, falling edge interrupt |
| **UNO 328P (slave)**   | Movement LED → D8 (PB0) <br>Door LED → D9 (PB1) <br>Buzzer → D3 (PD3 / Timer-1 toggle) <br>SPI SS PB2 input (kept high by master)               |                                                   |

_(Show CirkitStudio or Fritzing schematic in report.)_

---

## 2 Functional demo script

> **Tip:** Do the steps in this exact order; each line ticks a rubric box.

| Step                      | What to say / do                                   | What grader sees / hears                                                                                                                                           |
| ------------------------- | -------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **Idle**                  | Power up. “System waits for floor”.                | LCD: “Choose floor” (rubric 1.1).                                                                                                                                  |
| **Normal ride up**        | Enter **0 → 7**.                                   | • Movement LED ON (UNO).<br>• LCD second line: _Floor 00 01 … 07_ with leading zeroes.<br>• **Ding** on every new floor (extra-credit).                            |
| **Door case**             | Car reaches 7th floor.                             | Door LED ON 5 s, LCD “Door opening… / Door closed” (rubric 1.3).                                                                                                   |
| **Fault case**            | While on floor 7 enter **7 7**.                    | Movement LED blinks 3×, LCD returns to idle (rubric 1.5).                                                                                                          |
| **Ride down + emergency** | Enter 15 → 3. While moving press emergency button. | LCD “!!! EMERGENCY !!!”, 3× blink.<br>LCD prompts “Press # to open”. (Improved level ✔ +½ pt).<br>Press **#**: door LED ON 5 s, 11-note melody, door closes, idle. |

---

## 3 Software architecture talking points

```
+------------ MEGA ------------+      SPI (1-byte opcodes)       +----------- UNO ------------+
| main.c                        | --CMD_MOVEMENT_LED_*---> LED   | main.c                      |
|  FSM states: IDLE / MOVING    | --CMD_DOOR_LED_*-------> LED   |  SPI_STC_vect: LEDs, ding   |
|  / DOOR / EMERGENCY           | --CMD_BUZZER_PLAY_ONESHOT----> |  Timer-1 COMPA_vect: buzzer |
|  INT4_vect → emg_flag         | --CMD_DING---------------> |   |  play_melody(), play_ding() |
|  Timer-1 10 ms tick (ISR)     |                              |                              |
+--------------------------------+                              +-----------------------------+
```

- **Timer-1 CTC on MEGA** generates a 10 ms tick (`ISR(TIMER1_COMPA_vect)`), used by `wait_ms()` → fulfils “Use ISR” bonus.
- All long waits replaced by `wait_ms()`, so external IRQs stay responsive.
- Arrival “ding” uses new opcode `CMD_DING` (extra feature +½ pt).
- SPI is 1 MHz, fire-and-forget, simpler than I²C.

---

## 4 Rubric checklist & points projection

| Grading line                | Evidence in demo/code                                           | Expected pts |
| --------------------------- | --------------------------------------------------------------- | ------------ |
| **Functionality (0-20)**    | All 5 mandatory cases + ding                                    | **18-20**    |
| **Code structure (0-10)**   | Modular, FSM, ISR tick                                          | 8-10         |
| **Commenting (0-10)**       | Banner + per-func comments                                      | 8-10         |
| **Report (0-7)**            | Parts list, state-diagram, schematic, improvement notes         | 6-7          |
| **Schematic clarity (0-3)** | CirkitStudio PDF neat                                           | 3            |
| **Extra points (max 5)**    | • Improved emergency +½ <br>• ISR tick +1 <br>• Ding feature +½ | **+2 pts**   |

> **Total target:** ~ 47-50 + 2 extras ≈ **49-52 / 55** → well into the top band.

---

## 5 Quick Q & A

- **Why SPI?**: Only three wires + ground, no addressing overhead; 1 MHz safe at jumper length.
- **Timer overhead?**: 100 Hz tick uses < 1 % CPU.
- **Debounce?**: Keypad library samples at 10 ms and requires 3-sample stability.
- **What if emergency during door ?**: Button ignored because door state blocks movement—same as real lifts.
- **Future improvements?**: Multi-call queue, sleep after 30 s idle, PWM buzzer for softer tone.
