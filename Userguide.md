**Elevator Simulator – User Guide**  
_(last update 2025-05-xx)_

This project turns two Arduino boards into a miniature elevator controller.  
Follow the steps below to power-up, ride between floors, and trigger the special modes.

---

## 1 Quick layout

| Control / Indicator  | Location                                  | What it does                                                                     |
| -------------------- | ----------------------------------------- | -------------------------------------------------------------------------------- |
| **4 × 4 Keypad**     | MEGA board                                | Enter any two-digit floor `00-99`. `#` is the confirm key during emergency.      |
| **16 × 2 LCD**       | MEGA board                                | Shows prompts, floor numbers, door status, emergency messages.                   |
| **Emergency button** | MEGA pin **D2** (stand-alone push-button) | Stops the car immediately and enters emergency mode.                             |
| **Movement LED**     | UNO pin D8 (green)                        | ON while the car is travelling.                                                  |
| **Door LED**         | UNO pin D9 (blue)                         | ON when the door is open (normal or emergency).                                  |
| **Buzzer**           | UNO pin D3                                | Plays • a _ding_ each time a new floor is reached • a short melody in emergency. |

---

## 2 Normal operation

1. **Power-up**  
   _LCD shows:_

   ```
   Choose floor
   ```

2. **Select a destination**  
   _Press two digits_ (e.g. `0` `7` → floor 7).  
   _LCD second line updates in real-time:_

   ```
   Floor 00   ← leading zero
   Floor 01
   ...
   ```

3. **En-route**

   - Movement LED stays **ON**.
   - A short **ding** sounds **every new floor**.

4. **Arrival**
   - Door LED turns **ON** for 5 s.
   - LCD: `Door opening…` → `Door closed`.
   - System returns to _Choose floor_.

---

## 3 Special cases

| Case                           | How to trigger                              | System response                                                                                                                                                                                                        |
| ------------------------------ | ------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Fault (same floor)**         | Enter the current floor again               | Movement LED blinks **3×**, then idle.                                                                                                                                                                                 |
| **Improved emergency**         | Press the red emergency button while moving | 1. LCD _!!! EMERGENCY !!!_ <br>2. Movement LED blinks **3×**. <br>3. LCD asks **“Press # to open”**. <br>4. When you press `#` on the keypad → Door LED ON 5 s + buzzer melody, then door closes and system goes idle. |
| **New floor during emergency** | –                                           | Floor counter freezes; emergency overrides movement.                                                                                                                                                                   |

---

## 4 Key prompts and indicators

| LCD text            | Meaning / your action                 |
| ------------------- | ------------------------------------- |
| `Choose floor`      | Waiting for two-digit floor input.    |
| `Floor xx`          | Car is between floors.                |
| `Door opening…`     | Door LED is ON; wait.                 |
| `!!! EMERGENCY !!!` | Driver pressed emergency button.      |
| `Press # to open`   | Confirm door opening by pressing `#`. |

---

## 5 Troubleshooting

| Symptom                                   | Check                                                                                    |
| ----------------------------------------- | ---------------------------------------------------------------------------------------- |
| LCD stuck on _“Choose floor”_ after entry | Make sure you pressed **two digits**.                                                    |
| No ding / melody                          | Buzzer wire on UNO D3? Volume finger on piezo?                                           |
| Emergency button ignored                  | Button must short MEGA **D2 (PE4)** to **GND**; internal pull-up supplies 5 V when idle. |
| LEDs never light                          | Polarity, 330 Ω series resistor, or SPI cable between MEGA ↔ UNO.                        |

---

If you have questions, open an issue in the GitHub repo or contact the project team.
