# Elevator Control System

This project implements an elevator system using an **Arduino MEGA (master)** and an **Arduino UNO (slave)** communicating via SPI with ISR.

## Overview

- **Arduino MEGA (Master):**
  - **Input & Display:**  
    - Keypad for floor numbers (0–99)
    - LCD for displaying status (idle, current floor, door, emergency)
    - External emergency button  
  - **Logic:**  
    - Elevator movement (up/down, fault state)
    - Door control (open for 5 seconds)
    - Emergency sequence with appropriate notifications

- **Arduino UNO (Slave):**
  - **Peripheral Control:**  
    - Movement LED (on during movement, blinks 3 times on fault/emergency)
    - Door LED (indicates door open/close)
    - Buzzer (plays melody with at least 4 notes during emergencies)
  - **Communication:**  
    - Receives commands from the MEGA (optionally using ISRs for efficiency)

## Hardware & Software

- **Hardware:**  
  - Arduino MEGA (ATmega2560)  
  - Arduino UNO (ATmega328p)  
  - Keypad, LCD, 2 LEDs, Buzzer, External Emergency Button

- **Software:**  
  - Developed in Microchip Studio  
  - Uses provided libraries for LCD and keypad  
  - Implements SPI/I2C communication for board interaction


