# ECE 362 Mini Project — Cup Pong Game Console

## Project Overview

This project is a mini game console developed for **ECE 362: Microprocessor Systems and Interfacing** at Purdue University. Using the RP2350B microcontroller on the Purdue Proton development board, our team is building a top-down Cup Pong game that integrates multiple hardware peripherals.

The goal is to create an interactive game where players use a joystick to aim and throw a ball into a formation of cups displayed on an LCD screen. Successfully landing a ball in a cup removes it from the table, and the player attempts to clear all 10 cups in as few shots as possible.

The project incorporates SPI, ADC, PWM, and I2C communication to support graphics, player controls, audio feedback, and score storage.

## Hardware Components

| Component | Purpose | Interface |
|---|---|---|
| Purdue Proton (RP2350B) | Main microcontroller | — |
| 2.2" TFT LCD (ILI9341) | Game graphics and animations | SPI |
| Analog Joystick | Aiming and shooting controls | ADC / GPIO |
| Speaker | Game sound effects | PWM |
| EEPROM | High-score storage | I2C |

## Team Responsibilities

| Team Member | Responsibility |
|---|---|
| Adi | SPI LCD initialization, display driver, and Cup Pong graphics |
| Brandon | ADC joystick input and controls |
| Raghav | PWM speaker and sound effects |
| Joey | I2C EEPROM and score storage, KiCad Schematic and PCB Creation |

Each peripheral is developed separately before being integrated into the complete game.

## Gameplay

Cup Pong uses a top-down view of a table with 10 cups arranged in a triangle.

1. The player moves the joystick to position an aiming crosshair.
2. Pressing the joystick button launches a ball toward the selected location.
3. If the ball lands inside an active cup, the cup is removed.
4. The number of remaining cups and total shots is displayed on the LCD.
5. The round ends when all 10 cups are cleared.
6. The player's best score, measured by the fewest shots, can be saved using EEPROM.





## Building the Project

### Requirements

- Visual Studio Code
- PlatformIO extension
- Purdue Proton development board
- Raspberry Pi Pico SDK (configured through PlatformIO)

### Instructions

1. Clone the repository:

   ```bash
   git clone https://github.com/jsteinka05-ui/ECE362-Mini-Project-Gamer-Console.git
   ```

2. Open the repository folder in VS Code.

3. Allow PlatformIO to install the necessary dependencies.

4. Select the `proton` environment and run **PlatformIO: Build**.

5. Once the hardware is connected, upload the firmware to the Proton using the configured upload/debug interface.

The PlatformIO build has been completed successfully. LCD hardware testing is still pending.

