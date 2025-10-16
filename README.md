# Flight-simulator

Space Flight Simulator
A simple space flight simulator game running on an Atmel AVR microcontroller (e.g., ATmega328P). Control a spaceship with a joystick, collect stars, shoot asteroids, and avoid collisions! Displayed on a 128x64 SSD1306 OLED screen with sound effects via a buzzer.



Features
Gameplay: Control a spaceship to collect stars (5 points each) and shoot asteroids (10 points each). Avoid collisions to keep playing!

Hardware:

Atmel AVR microcontroller (tested on ATmega328P).

SSD1306 OLED display (128x64, I2C).

Joystick (connected to ADC0 and ADC1 for X/Y).

Push buttons (PD2 for shooting, PD4 for reset).

Buzzer (PD3 for sound effects).

Tech:

Written in C++ using AVR-GCC.

60 FPS game loop with timer interrupts.

I2C for OLED, ADC for joystick, PWM for buzzer.

Hardware Setup
Components
ATmega328P (or similar AVR microcontroller).

SSD1306 OLED display (I2C: SDA to PC4, SCL to PC5).

Analog joystick (X to PC0/ADC0, Y to PC1/ADC1).

Two push buttons (PD2 for shoot, PD4 for reset, with pull-up resistors).

Buzzer (PD3 for PWM output).

16 MHz crystal (or internal oscillator).

Schematic
[Insert schematic diagram here, e.g., created with Fritzing or KiCad]
Installation
Clone the repository:

git clone https://github.com/rpnabedi-art/Flight-simulator.git
Set up AVR toolchain:

Install AVR-GCC or use Atmel Studio.

Optionally, use PlatformIO for Arduino compatibility.

Compile and upload:

Compile SpaceFlightSimulator.cpp with AVR-GCC.

Flash the .hex file to your AVR using avrdude:

avrdude -c usbasp -p atmega328p -U flash:w:SpaceFlightSimulator.hex
Connect hardware as per the schematic.

Run: Power on the AVR, and the game starts automatically!

How to Play
Joystick: Move the spaceship (up/down/left/right).

Shoot Button (PD2): Fire lasers to destroy asteroids.

Reset Button (PD4): Restart the game after "Game Over."

Objective: Collect stars for points, shoot asteroids, and avoid collisions.

Score: Stars = 5 points, Asteroids = 10 points.

Future Improvements
Add a frame buffer for faster OLED rendering.

Implement text display for score and "Game Over" (requires font library).

Connect to PC-based simulators (e.g., MSFS 2024) via UART/USB-HID.

Add more complex sprites and animations.

Improve sound effects with non-blocking PWM.

Contributing
Feel free to fork this repo and submit pull requests! Ideas:

Add support for MPU6050 gyroscope for tilt control.

Create a PC client (Python with pySerial) to visualize data.

Expand to multiplayer or leaderboard.

License
MIT License

Acknowledgments
Inspired by classic games like Asteroids.

Thanks to the AVR community for awesome tutorials and libraries!

