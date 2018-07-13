# Motor driver for the Brixx project

## About

... Details will follow, I just wanted to upload the circuit since the project is not one of my top prioritized at the moment

## Parts

For X motor connectors.

| Amount | Part | Notes |
| - | - | - |
| X | motor driver IC | I used the TLE 4202B |
| X | 100nF ceramic capacitor | - |
| X | 220nF ceramic capacitor | - |
| X | 100µF electrolytic capacitor | - |
| 1 | diode | This one is critical! |
| X/2+1 | LEGO Power Functions extension cable | LEGO online store. Cut them it in the middle. |

## Circuit

![motor-driver-circuit](/Media/motor-driver-circuit.png)

The whole device (Arduino as well as any motors and this circuit) will be powered by a standard LEGO Power Functions battery pack.
Be sure to use one of the dark ends of LEGO Power Functions extension cable for the power connection (the light ones won't fit).
The diode will protect your Arduino board and probably your USB port if you upload new sketches while motors are connected but LEGO power supply is off.

Connect the control pins of the motor drivers to Arduino's PWM pins.
If you don't have enough go with the digital pins, but be aware that you won't be able control the motor's speed then.
