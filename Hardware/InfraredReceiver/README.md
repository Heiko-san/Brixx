# Infrared receiver for the Brixx project

## About

This is a classic infrared receiver circuit which will be used to receive commands from the LEGO Power Functions
remote contols using the `PowerFunctionsIR` lib of this project.

## Parts

| Amount | Part | Notes |
| - | - | - |
| 1 | Infrared receiver module | I used the OS-OPTO OS-1638 |
| 1 | 100nF ceramic capacitor | - |
| 1 | 47µF electrolytic capacitor | - |
| 1 | 47Ω resistor | - |
| 1 | 22kΩ resistor | Actually the datasheet says >10kΩ, this is a pullup resistor, so bigger values make for less current flow. |
| 1 | Small piece of circuit board | - |
| 1 | 3-wire cable | Like servo wire, etc. |


## Circuit

![IR-receiver-circuit](/Media/IR-receiver-circuit.png)

The power supply of this circuit gets connected to Arduino `+5V` and `GND` pins. The output pin goes to `pin18` on Arduino Mega.

> You can use any other `external interrupt` pin for this circuit's output line aswell.
> Especially if you use a board other then Arduino Mega, you will have to do this.
> However you will need to redefine `IR_SAMPLE_INTERRUPT_PIN` in `BrixxSettings.h` then.

The circuit is almost exactly as described in the datasheet for the OS-1638, only that I added a 100nF capacitor to smooth
the input voltage. This 100nF capacitor should go as close as possible to the pinout of the IR receiver module.

The circuit should work for any other IR receiver aswell, just watch out for the pinout since it may differ, you should
have a look into the datasheet if in doubt.
