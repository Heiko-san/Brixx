/*
    PowerFunctionsIR.h - Library to read LEGO Power Fundtions remote controls
    Created by Heiko Finzel, 2017.

    * with init() (in setup() function) idle_isr() is attached and it watches for start-stop-signal
    * if start-stop-signal is detected sample_isr() is attached and samples the signal data
    * switch back to idle_isr() on signal end or sample error
    * if 16 bits were sampled successfully the sample value is enqueued for further processing as IRSample struct
    * queue is polled by update() (in loop() function) and event handlers are triggered
*/
#pragma once
#include "Arduino.h"
#include "BrixxSettings.h"

namespace PowerFunctionsIR {

// sample bit timings
#define LOW_MIN             316
#define HIGH_MIN            526
#define START_STOP_MIN      947
#define START_STOP_MAX     1560
// ...
#define RED                 0x1
#define BLUE                0x2


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

    // determining sampling source
    uint8_t get_channel() const { return get_zero_based_channel() + 1; } // convert channels to 1-8
    uint8_t get_single_output_port() const { return mode & 0x1; }
    // determining the sending mode
    bool combo_direct_mode() const { return !escape && mode == 1; } // this one is used by the standard LEGO remote control
    bool single_output_mode_pwm() const { return single_output_mode() && mode ^ 0x2; } // this one is used by the pwm LEGO remote control
    bool single_output_mode() const { return !escape && mode & 0x4; } // the other modes seem to be unused ...
    bool single_output_mode_toggle() const { return single_output_mode() && mode & 0x2; }
    bool extended_mode() const { return !escape && mode == 0; }
    bool combo_pwm_mode() const { return escape; }
    // helper functions for internal use
    bool checksum_ok() const { return (0xF ^ nibble1 ^ nibble2 ^ nibble3) == nibble4; }
    uint8_t get_zero_based_channel() const { return address << 2 | channel; } // convert address space 1 to channels 4-7
};

/*
    Event processing
*/
// event handler definition
typedef void (*EventHandler)(IRSample&);
// event handlers - generic_handler is called for every type of event
extern EventHandler generic_handler;

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
