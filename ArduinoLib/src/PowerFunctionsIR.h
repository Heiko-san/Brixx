/*
 * PowerFunctionsIR.h - Library to read LEGO Power Fundtions remote controls
 * Copyright© 2017 by Heiko Finzel
 * The Brixx Library and all other content of this project is distributed under the terms and conditions
 * of the GNU GENERAL PUBLIC LICENSE Version 3.
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without
 * damaging anything else then just a few extension cables, I'll take no responsibility if it happens
 * afterall. Your warranty for your LEGO Power Functions items may void using this project.
 *
 *   - with init() (in setup() function) idle_isr() is attached and it listens for start-stop-signal
 *   - if start-stop-signal is detected sample_isr() is attached and samples the signal data
 *   - switch back to idle_isr() on signal end or sample error
 *   - if 16 bits were sampled successfully the sample value is enqueued for further processing as IRSample struct
 *   - queue is polled by update() (in loop() function) and event handlers are triggered
 */
#pragma once
#include "Arduino.h"
#include "BrixxSettings.h"

namespace PowerFunctionsIR {

// sample bit timings
#define LOW_MIN                      316
#define HIGH_MIN                     526
#define START_STOP_MIN               947
#define START_STOP_MAX              1560
// forward = clockwise
#define RED_STOP_BLUE_STOP          0x00
#define RED_FORWARD_BLUE_STOP       0x01
#define RED_BACKWARD_BLUE_STOP      0x02
#define RED_STOP_BLUE_FORWARD       0x04
#define RED_FORWARD_BLUE_FORWARD    0x05
#define RED_BACKWARD_BLUE_FORWARD   0x06
#define RED_STOP_BLUE_BACKWARD      0x08
#define RED_FORWARD_BLUE_BACKWARD   0x09
#define RED_BACKWARD_BLUE_BACKWARD  0x0A
// increase = more clockwise
#define INCREASE_VALUE              0x40
#define DECREASE_VALUE              0x50
#define RESET_VALUE                 0x80

 


/*
    IRSample struct/union
    "protocol wrapper" around the sampled data
*/
struct IRSample {
    union {
        volatile uint16_t raw;
        const struct {
            uint8_t nibble4:    4;
            uint8_t nibble3:    4;
            uint8_t nibble2:    4;
            uint8_t nibble1:    4;
        };
        const struct {
            uint8_t checksum:   4;
            uint8_t data:       4;
            uint8_t mode:       3;
            uint8_t address:    1;
            uint8_t channel:    2;
            uint8_t escape:     1;
            uint8_t toggle:     1;
        };
    };
    bool handled = false;

    // user interface
    uint8_t get_channel() const { return channel + 1; } // convert channels to 1-4
    bool standard_rc() const { return combo_direct_mode(); }
    bool pwm_rc() const { return single_output_mode_pwm(); }
    bool red_effected() const { return standard_rc() ? true : get_single_output_port() == 0; } // standard rc always effects both
    bool blue_effected() const { return standard_rc() ? true : get_single_output_port() == 1; }
    uint8_t get_command() const { return standard_rc() ? data : data << 4; }
    /*uint8_t get_extened_channel() const { return (address << 2 | channel) + 1; } // convert channels to 1-8 (5-8 not used yet)*/
    // helper functions for internal use
    bool checksum_ok() const { return (0xF ^ nibble1 ^ nibble2 ^ nibble3) == nibble4; }
    uint8_t get_state_signature() const { return data << 4 | mode << 3 | toggle; }
    // determining the sending mode
    bool combo_direct_mode() const { return !escape && mode == 1; } // this one is used by the standard LEGO remote control
    bool single_output_mode_pwm() const { return single_output_mode() && mode ^ 0x2; } // this one is used by the pwm LEGO remote control
    bool single_output_mode() const { return !escape && mode & 0x4; } // the other modes seem to be unused ...
    bool single_output_mode_toggle() const { return single_output_mode() && mode & 0x2; }
    bool extended_mode() const { return !escape && mode == 0; }
    bool combo_pwm_mode() const { return escape; }
    uint8_t get_single_output_port() const { return mode & 0x1; }
};

/*
    Remember the signature of last event to filter out redundancy signals
    and remember "output values" for user convenience.
*/
struct ChannelState {
    uint8_t previous;
    struct {
        int16_t value: 9;
        uint8_t step: 7;
    } red;
    struct {
        int16_t value: 9;
        uint8_t step: 7;
    } blue;
};

/*
    Event processing
*/
// event handler definition
typedef void (*EventHandler)(IRSample&, ChannelState&);
// event handlers - generic_handler is called for every type of event
extern EventHandler generic_handler;
extern ChannelState channel_states[4];

// event queue element
struct Event {
    IRSample ir_event;
    Event* next;
};

// add elements to the queue
void enqueue(IRSample sample);
// remove elements from the queue
IRSample dequeue( void );
// call PowerFunctionsIR::update() in loop() to process IR events
void update( void );

/*
    Sampling
*/
// interrupt service routing for sampling
void sample_isr( void );
// interrupt service routing while not sampling
void idle_isr( void );
// call PowerFunctionsIR::init() in setup() if you want to use remote control
void init( void );

}; // end namespace
