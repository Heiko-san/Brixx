/*
 * PowerFunctionsIR.h - Library to read LEGO Power Fundtions remote controls
 * Copyright© 2017 by Heiko Finzel                                                                                      
 * The Brixx Library and all other content of this project is distributed under the terms and conditions of the         
 * GNU GENERAL PUBLIC LICENSE Version 3.                                                                                
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without damaging anything else  
 * then just a few extension cables, I'll take no responsibility for any damage done to any of your hardware.           
 * Warranty for your LEGO Power Functions items may void using this project.
 *
 *   - With init() (in setup() function) idle_isr() is attached and listens for start-stop-signal.
 *   - If start-stop-signal is detected sample_isr() is attached and samples the signal data.
 *   - Switch back to idle_isr() on signal end or sample error.
 *   - If 16 bits were sampled successfully the sample value is enqueued for further processing as IRSample struct.
 *   - Queue is polled by update() (in loop() function) and event handlers are triggered.
 */
#pragma once
#include "Arduino.h"
#include "BrixxSettings.h"

namespace PowerFunctionsIR {

/*
    Number of supported channels.
    This is used for arrays, where 1 element per channel is required, e.g. event handlers.
*/
#define NUMBER_CHANNELS                4
/*
    Sample bit timings.
    LOW bit         = 316 -  526 µs (typically  421µs)
    HIGH bit        = 526 -  947 µs (typically  711µs)
    START-STOP bit  = 947 - 1560 µs (typically 1184µs)
*/
#define LOW_MIN                      316
#define HIGH_MIN                     526
#define START_STOP_MIN               947
#define START_STOP_MAX              1560
/*
    Command codes for LEGO Power Functions standard remote control.
    FORWARD  = clockwise         =  255
    BACKWARD = counter clockwise = -255
    STOP     = no movement       =    0
*/
#define RED_STOP_BLUE_STOP          0x00
#define RED_FORWARD_BLUE_STOP       0x01
#define RED_BACKWARD_BLUE_STOP      0x02
#define RED_STOP_BLUE_FORWARD       0x04
#define RED_FORWARD_BLUE_FORWARD    0x05
#define RED_BACKWARD_BLUE_FORWARD   0x06
#define RED_STOP_BLUE_BACKWARD      0x08
#define RED_FORWARD_BLUE_BACKWARD   0x09
#define RED_BACKWARD_BLUE_BACKWARD  0x0A
/*
    Command codes for LEGO Power Functions pwm remote control.
    INCREASE = more clockwise         = +1 step
    DECREASE = more counter clockwise = -1 step
    RESET    = no movement            =  0
*/
#define INCREASE_VALUE              0x40
#define DECREASE_VALUE              0x50
#define RESET_VALUE                 0x80
/*
    Pseudo command code for LEGO Power Functions pwm remote control.
    If get_red/blue_command is queried for wrong channel.
*/
#define NO_COMMAND                  0xFF
/*
    Derived command codes for LEGO Power Functions standard remote control.
    Used for "single channel commands" in get_red/blue_command.
*/
#define STOP                        0x00
#define FORWARD                     0x01
#define BACKWARD                    0x02

/*
    IRSample struct/union - "protocol wrapper" around the sampled data.
*/
struct IRSample {
    // Data ...
    union {
        // Raw field 16 bits for sampling.
        volatile uint16_t raw;
        // Nibble fields for checksum calculation.
        const struct {
            uint8_t nibble4:    4;
            uint8_t nibble3:    4;
            uint8_t nibble2:    4;
            uint8_t nibble1:    4;
        };
        // Protocol related fields for "named" access.
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
    // Handled field for event handlers to indicate if this sample should be passed on to further event handlers.
    bool handled = false;
    /*
        User interface functions for use in event handlers.
    */
    // Get the remote control channel (0-3 = 1-4).
    uint8_t get_channel( void ) const { return channel; }
    /* Convert channels to 0-7 (4-7 not used yet).
    uint8_t get_channel( void ) const { return address << 2 | channel; } */
    // Check if command was sent by LEGO Power Functions standard remote control (with joysticks).
    bool standard_rc( void ) const { return combo_direct_mode(); }
    // Check if command was sent by LEGO Power Functions pwm remote control (with speed control wheels).
    bool pwm_rc( void ) const { return single_output_mode_pwm(); }
    // Check if command effects red/blue subchannel (standard rc always effects both).
    bool red_effected( void ) const { return standard_rc() ? true : get_single_output_port() == 0; }
    bool blue_effected( void ) const { return standard_rc() ? true : get_single_output_port() == 1; }
    // Convert the actual command to an unique command code (see definitions above).
    uint8_t get_command( void ) const { return standard_rc() ? data : data << 4; }
    // Convert the actual command to an unique command code only for red/blue subchannel.
    uint8_t get_red_command( void ) const
        { return standard_rc() ? data & 0x3 : get_single_output_port() == 0 ? data << 4 : NO_COMMAND; }
    uint8_t get_blue_command( void ) const
        { return standard_rc() ? data >> 2 & 0x3 : get_single_output_port() == 1 ? data << 4 : NO_COMMAND; }
    /*
        Helper functions for internal use only.
    */
    // Check the checksum.
    bool checksum_ok( void ) const { return (0xF ^ nibble1 ^ nibble2 ^ nibble3) == nibble4; }
    // A signature from data, mode and toggle bit to identify redundancy commands.
    uint8_t get_state_signature( void ) const { return data << 4 | mode << 3 | toggle; }
    // Used for single_output_mode_pwm (pwm rc) to get the effected subchannel (red/blue).
    uint8_t get_single_output_port( void ) const { return mode & 0x1; }
    /*
        Determining the sending mode.
    */
    // This one is used by the LEGO Power Functions standard remote control (with joysticks).
    bool combo_direct_mode( void ) const { return !escape && mode == 1; }
    // This one is used by the LEGO Power Functions pwm remote control (with speed control wheels).
    bool single_output_mode_pwm( void ) const { return single_output_mode() && mode ^ 0x2; }
    // The other modes seem to be unused.
    bool single_output_mode( void ) const { return !escape && mode & 0x4; }
    bool single_output_mode_toggle( void ) const { return single_output_mode() && mode & 0x2; }
    bool extended_mode( void ) const { return !escape && mode == 0; }
    bool combo_pwm_mode( void ) const { return escape; }
};

/*
    ChannelState struct.
    Remember the signature of last event to filter out redundant signals (previous).
    Keep track of internal values effected by IR events (actual_step, value()).
    Remember the steps set for each channel and subchannel (steps).
*/
struct ChannelState {
    uint8_t previous;
    struct {
        int8_t actual_step;
        uint8_t steps: 7;
        uint8_t alternative: 1;
        int16_t value( void ) const { return alternative ? map(actual_step, 0, steps, 0, 255) : 
            map(actual_step, -steps, steps, -255, 255); } 
        uint8_t bit_switches( void ) const { return actual_step & 0x03; }
    } red;
    struct {
        int8_t actual_step;
        uint8_t steps: 7; // max steps 127
        uint8_t alternative: 1;
        int16_t value( void ) const { return alternative ? map(actual_step, 0, steps, 0, 255) : 
            map(actual_step, -steps, steps, -255, 255); } 
        uint8_t bit_switches( void ) const { return actual_step & 0x03; }
    } blue;
};

/*
    Sampling IR events using an external interrupt pin.
*/
// Interrupt service routing while not sampling (attached by init() or sample_isr()).
void idle_isr( void );
// Interrupt service routing for sampling (attached by idle_isr()).
void sample_isr( void );

/*
    Event processing.
*/
// Event handler definition.
typedef void (*EventHandler)(IRSample&, ChannelState&);
// Event handlers - generic_handler is called for every type of event.
extern EventHandler generic_handler;
// These are called if the corresponding channel and subchannel is effected (channels = index 0-3).
extern EventHandler red_effected_handler[NUMBER_CHANNELS];
extern EventHandler blue_effected_handler[NUMBER_CHANNELS];
// These are called if the value associated with the channel and subchannel changes (channels = index 0-3).
extern EventHandler red_changed_handler[NUMBER_CHANNELS];
extern EventHandler blue_changed_handler[NUMBER_CHANNELS];
/*
    Event struct - event queue element.
    Consists of IRSample and pointer to the next Event.
*/
struct Event {
    IRSample ir_event;
    Event* next;
};
// Add an element to the queue.
void enqueue(IRSample &sample);
// Remove the first element from the queue.
Event* dequeue( void );

/*
    User interface functions.
*/
// Call PowerFunctionsIR::init() in setup() if you want to use remote control.
void init( void );
// Call PowerFunctionsIR::update() in loop() to process IR events.
void update( void );
// This can be used to change the steps attributes of the channel's ChannelState.
bool set_steps(uint8_t channel, uint8_t steps_red, uint8_t steps_blue);
// This can be used to change value tracking to "alternative mode" (0-225 / 2 bit on-off-switch)
bool set_alternative_mode(uint8_t channel, bool red, bool blue);
// Get the ChannelState for channel.
ChannelState get_state_for_channel(uint8_t channel);

}; // end namespace
