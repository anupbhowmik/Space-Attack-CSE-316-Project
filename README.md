# Space Attack-Game on Microcontroller

A simple game playable using button interface as controller and LED Matrix as the display. Includes an innovative approach that takes advantage of a Shift Register to make the interface efficient.

# Contents

- `C` Codes that can be compiled in `Mircrochip Studio` to generate `hex files`. The hex files can be burned into any `ATMega32`
- Simulation projects that can be simulated in `Proteus`

## Circuit Diagram

![Circuit Diagram](space-ttack.BMP)

## How the Game Works

An `ATMega32` controls the main logic of the game. It also provides outputs
required to run the 3x2 LED Matrix (each LED Matrix is 8x8) setup. There are two buttons attached to PA3(Player Goes up)
and PA4(Player Goes down). Another button (PA5) is for shooting bullets. <br>
There is also an `LCD display` that keeps track of the current score and health
remaining.

## Problems faced and solution

To achieve a better gaming experience, we had decided to stack two rows of
three 8x8 LED Matrices together and make a 3x2 display. Every single color
LED Matrix has 16 pins, therefore, in order to control the 6 LED Matrices, we
would need `16 × 6 = 96` pins along with the control pins for the LCD Display. Connecting all those pins would require a lot of wires and there is not enough pins avaliable in ATMega32. <br>

That’s why we came up with a clever solution to minimize the number of pins
required to control the LED Matrices and with this trick, the number of
necessary pins can be kept constant and the display can be extended to `any
number of LED Matrices`. <br>
The trick is to use `74HC595N Shift Register` (6 Shift Registers for 6 LED Matrices) to control the voltage given
to the rows of the LED Matrices.

### Detailed Solution: Row-Column Multiplexing

Instead of lighting all LEDs at once, we light one row at a time, very quickly. Kind of like top to bottom scanning and refreshing the display. Here are the steps:

1. Set Row 0 HIGH, all other rows LOW
1. Set columns according to desired pattern for Row 0
1. Brief delay (1ms in the code)
1. Move to Row 1, repeat process (the Shift Register carries the 1 forward for each of the rows)

Continue through all 8 rows Repeat entire cycle continuously. Because this happens so fast, your eyes see a complete image due to persistence of vision.

### Pin Efficiency

3 control pins for shift registers (Data, Clock, Latch). The Shift Registers are `daisy-chained`, so only 3 pins are required to control all 6 LED Matrices.
8 pins for column data (PORTB)
Total: 11 pins instead of 96 pins

### Speed and Smoothness

- Complete display refresh: 6 matrices × 8 rows × 1ms = 48ms
- Refresh rate: ~21 Hz (smooth enough for gaming)
