
## Requirements:

**Arduino Mega (Master):**
- **Input & Display:**  
  - Read floor numbers from keypad (0–99).  
  - Show messages/status on LCD (idle, floor, door status, emergency).  
- **Control & Logic:**  
  - Manage elevator movement (up/down decisions, fault state if floor unchanged).  
  - Detect emergency via an external button and trigger emergency sequences.  
- **Communication:**  
  - Send commands over SPI/I2C to the UNO regarding movement, door, and emergency actions.

**Arduino Uno (Slave):**
- **Peripheral Control:**  
  - Control movement LED (on during movement, blink for fault/emergency).  
  - Control door LED (turn on for 5 seconds to indicate door open/close).  
  - Play a buzzer melody (min. 4 notes) during emergencies.
- **Communication:**  
  - Receive and interpret commands from the MEGA via SPI/I2C (using ISR optionally for efficiency).



### implemented so far:

---

- Wiring and hardware

---

- MEGA keypad, screen
- MEGA SPI ISR send

---

- uno SPI ISR Recieve

### todo:

- uno libraries
- uno leds and buzzer control.
- Sending of correct data with SPI.

