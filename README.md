# Space-Attack-CSE-316-Project

# Contents
- `C` Codes that can be compiled in `Mircrochip Studio` to generate `hex files`. The hex files can be burned into any `ATMega32`
-  Simulation projects that can be simulated in `Proteus`

## Circuit Diagram
![](https://github.com/Anupznk/Space-Attack-CSE-316-Project/blob/master/Space%20Attack.BMP)

## How the Game Works
An `ATMega32` controls the main logic of the game. It also provides outputs
required to run the 3x2 LED Matrix (each LED Matrix is 8x8) setup. There are two buttons attached to PA3(Player Goes up)
 and PA4(Player Goes down). Another button (PA5) is for shooting bullets. <br>
There is also an `LCD display` that keeps track of the current score and health
remaining.

## Problems faced and solutions
To achieve a better gaming experience, we had decided to stack two rows of
three 8x8 LED Matrices together and make a 3x2 display. Every single color
LED Matrix has 16 pins, therefore, in order to control the 6 LED Matrices, we
would need `16 × 6 = 96` pins along with the control pins for the LCD Display. Connecting all those pins would require a lot of wires and
the overall design wouldn’t be efficient at all. <br>

That’s why we came up with a clever solution to minimize the number of pins
required to control the LED Matrices and with this trick, the number of
necessary pins can be kept constant and the display can be extended to `any
number of LED Matrices`. <br>
The trick is to use `74HC595N Shift Register` (6 Shift Registers for 6 LED Matrices) to control the voltage given 
to the rows of the LED Matrices
