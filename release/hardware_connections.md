# Smart Shopping Trolley Hardware Connections

This document outlines the hardware wiring for the ESP32 (38-pin version) used in the Smart Shopping Trolley project.

## 1. MFRC522 RFID Reader (VSPI)
The RFID reader uses the default VSPI pins on the ESP32.
*   **SDA (SS)**  -> GPIO 5
*   **SCK**       -> GPIO 18
*   **MOSI**      -> GPIO 23
*   **MISO**      -> GPIO 19
*   **RST**       -> GPIO 27
*   **3.3V**      -> 3.3V on ESP32
*   **GND**       -> GND on ESP32

## 2. I2C LCD Display (16x2)
The LCD uses standard I2C pins.
*   **SDA** -> GPIO 21
*   **SCL** -> GPIO 22
*   **VCC** -> VIN (5V) on ESP32
*   **GND** -> GND on ESP32

## 3. Active Buzzer
*   **Positive (Longer Leg)** -> GPIO 26
*   **Negative (Shorter Leg)** -> GND on ESP32

## 4. Power Supply
Ensure the ESP32 is powered via a stable 5V source (USB or VIN) capable of providing enough current for the board, LCD, and RFID module simultaneously.
