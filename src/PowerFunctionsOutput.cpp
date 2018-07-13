#include "PowerFunctionsOutput.h"

PowerFunctionsOutput::PowerFunctionsOutput(uint8_t pin_c1, uint8_t pin_c2)
{
    _pin_c1 = pin_c1;
    _pin_c2 = pin_c2;
    pinMode(_pin_c1, OUTPUT);
    pinMode(_pin_c2, OUTPUT);
    off();
}

void PowerFunctionsOutput::c1_set(uint8_t value)
{
    analogWrite(_pin_c1, value);
    _value_c1 = value;
}

void PowerFunctionsOutput::c2_set(uint8_t value)
{
    analogWrite(_pin_c2, value);
    _value_c2 = value;
}

void PowerFunctionsOutput::set(uint8_t value_c1, uint8_t value_c2)
{
    c1_set(value_c1);
    c2_set(value_c2);
}
