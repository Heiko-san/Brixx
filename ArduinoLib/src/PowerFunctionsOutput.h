/*
  PowerFunctionsOutput.h - Base class for the LEGO Power Functions output ports (for motors, servos, etc.).
  Created by Heiko Finzel, 2017.
*/
#pragma once
#include "Arduino.h"
#include "BrixxSettings.h"

class PowerFunctionsOutput
{
  public:
    PowerFunctionsOutput( uint8_t pin_c1, uint8_t pin_c2 );
    ~PowerFunctionsOutput( void ) { off(); }
    // setters for c1
    void c1_set ( uint8_t value );
    void c1_on  ( void ) { c1_set(255); }
    void c1_off ( void ) { c1_set(0); }
    // setters for c2
    void c2_set ( uint8_t value );
    void c2_on  ( void ) { c2_set(255); } 
    void c2_off ( void ) { c2_set(0); }
    // setters both
    void set( uint8_t value_c1, uint8_t value_c2 );
    void off( void ) { set(0,0); }
    // getters for c1
    uint8_t c1_get( void ) const { return _value_c1; }
    bool c1_is_off( void ) const { return _value_c1 == 0; }
    // getters for c2
    uint8_t c2_get( void ) const { return _value_c2; }
    bool c2_is_off( void ) const { return _value_c2 == 0; }
    // getters both
    bool is_off( void ) const { return c1_is_off() && c2_is_off(); }
  private:
    uint8_t _pin_c1;
    uint8_t _pin_c2;
    uint8_t _value_c1 = 0;
    uint8_t _value_c2 = 0;
};
