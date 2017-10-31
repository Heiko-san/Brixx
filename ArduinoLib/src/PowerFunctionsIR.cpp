/*
 * PowerFunctionsIR.cpp - Library to read LEGO Power Fundtions remote controls
 * Copyright© 2017 by Heiko Finzel                                                                                      
 * The Brixx Library and all other content of this project is distributed under the terms and conditions of the         
 * GNU GENERAL PUBLIC LICENSE Version 3.                                                                                
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without damaging anything else  
 * then just a few extension cables, I'll take no responsibility for any damage done to any of your hardware.           
 * Warranty for your LEGO Power Functions items may void using this project.
 */
#include "PowerFunctionsIR.h"

namespace PowerFunctionsIR {

/*
    Variables.
*/
// Sampling buffer.
IRSample sample_value;
// Remember last sample interrupt time.
volatile unsigned long sample_micros_last = 0;
// Position of the bit to sample next.
volatile int8_t sample_position = 15;
// Our interrupt address (derived from interrupt pin in init() and saved here).
uint8_t interrupt_address;
// Saved states for all channels, see ChannelState struct in header.
ChannelState channel_states[NUMBER_CHANNELS];
// Event handlers - generic_handler is called for every type of event.                                                  
EventHandler generic_handler;
// These are called if the corresponding channel and subchannel is effected (channels = index 0-3).                     
EventHandler red_effected_handler[NUMBER_CHANNELS];
EventHandler blue_effected_handler[NUMBER_CHANNELS];
// These are called if the value associated with the channel and subchannel changes (channels = index 0-3).             
EventHandler red_changed_handler[NUMBER_CHANNELS];
EventHandler blue_changed_handler[NUMBER_CHANNELS];
// The event queue's first and last element.
Event* head;
Event* tail;

/*                                                                                                                      
    Sampling IR events using an external interrupt pin.                                                                 
*/
void idle_isr( void ) {
    unsigned long micros_now = micros();
    uint16_t micros_diff = micros_now - sample_micros_last;
    sample_micros_last = micros_now;
    if (micros_diff < START_STOP_MAX && micros_diff > START_STOP_MIN) {
        // Start sampling on start-stop-signal.
        sample_value.raw = 0;
        sample_position = 15;
        attachInterrupt(interrupt_address, sample_isr, FALLING);
    }
}

void sample_isr( void ) {
    unsigned long micros_now = micros();
    uint16_t micros_diff = micros_now - sample_micros_last;
    sample_micros_last = micros_now;
    if (micros_diff < START_STOP_MAX && micros_diff > LOW_MIN) {
        if (micros_diff > START_STOP_MIN) {
            // Restart sampling on early start-stop-signal (most certainly a new, interfering signal).
            sample_value.raw = 0;
            sample_position = 15;
        } else if (micros_diff > HIGH_MIN) {
            // Sampled a high bit.
            sample_value.raw |= 1 << sample_position;
            sample_position -= 1;
        } else {
            // Sampled a low bit (add 0 << sample_position => no change).
            sample_position -= 1;
        }
        if (sample_position < 0) {
            // Successfully sampled 16 bit -> enqueue event.
            enqueue(sample_value);
            attachInterrupt(interrupt_address, idle_isr, FALLING);
        }
    } else {
        // Reset on signal error.
        attachInterrupt(interrupt_address, idle_isr, FALLING);
    }
}

/*                                                                                                                      
    Event processing.                                                                                                   
*/
void enqueue(IRSample sample) {
    Event* p_event = (Event*)malloc(sizeof(Event));
    p_event->ir_event = sample;
    p_event->next = 0;
    if (tail) {
        tail->next = p_event;
        tail = p_event;
    } else {
        head = tail = p_event;
    }
}

IRSample dequeue( void ) {
    Event* p_event = head;
    /*
        If we remove the last element of the queue this is a critical action.
        To ensure queue doesn't get corrupted if interrupt adds an element at the
        wrong moment we lock interrupts until queue is in a consistent state again.
    */
    if (head == tail) noInterrupts();
    head = head->next;
    if (!head) tail = 0;
    // Unlock ...
    interrupts();
    IRSample sample_to_return = p_event->ir_event;
    free(p_event);
    return sample_to_return;
}

/*
    User interface functions.
*/
void init( void ) {
    sample_value.raw = 0;
    for (int i = 0; i < NUMBER_CHANNELS; i++) {
        channel_states[i].red.steps = DEFAULT_STEPS;
        channel_states[i].blue.steps = DEFAULT_STEPS;
    }
    pinMode(IR_SAMPLE_INTERRUPT_PIN, INPUT);
    interrupt_address = digitalPinToInterrupt(IR_SAMPLE_INTERRUPT_PIN);
    attachInterrupt(interrupt_address, idle_isr, FALLING);
}

void update( void ) {
    while ( head ) {
        IRSample ir = dequeue();
        /*
            We can't rely on toggle bit only for redundancy check, since there are edge cases where it doesn't work.
            * First signal received could be 0 or 1.
            * When going from full forward to full backward on standard rc the stop signal will be missed and
              therefore both signals have same toggle bit.
            * We could use standard rc and pwm rc on the same channel and differentiate them by mode.
            We still have an edge case: If first signal is on toggle 0 with data 0 and extended mode (0)
            we will miss it, but in reality extended mode isn't in use.
        */
        if (ir.checksum_ok() && channel_states[ir.get_channel()].previous != ir.get_state_signature()) {
            channel_states[ir.get_channel()].previous = ir.get_state_signature();
            // Values updated here.
            int8_t old_red_value = channel_states[ir.get_channel()].red.actual_step;
            switch (ir.get_red_command()) {
                case STOP:
                case RESET_VALUE:
                    channel_states[ir.get_channel()].red.actual_step = 0;
                    break;
                case FORWARD:
                    channel_states[ir.get_channel()].red.actual_step = channel_states[ir.get_channel()].red.steps;
                    break;
                case BACKWARD:
                    channel_states[ir.get_channel()].red.actual_step = -channel_states[ir.get_channel()].red.steps;
                    break;
                case INCREASE_VALUE:
                    if (channel_states[ir.get_channel()].red.actual_step <
                        channel_states[ir.get_channel()].red.steps)
                            channel_states[ir.get_channel()].red.actual_step++;
                    break;
                case DECREASE_VALUE:
                    if (channel_states[ir.get_channel()].red.actual_step >
                        -channel_states[ir.get_channel()].red.steps)
                            channel_states[ir.get_channel()].red.actual_step--;
                    break;
            }
            int8_t old_blue_value = channel_states[ir.get_channel()].blue.actual_step;
            switch (ir.get_blue_command()) {
                case STOP:
                case RESET_VALUE:
                    channel_states[ir.get_channel()].blue.actual_step = 0;
                    break;
                case FORWARD:
                    channel_states[ir.get_channel()].blue.actual_step = channel_states[ir.get_channel()].blue.steps;
                    break;
                case BACKWARD:
                    channel_states[ir.get_channel()].blue.actual_step = -channel_states[ir.get_channel()].blue.steps;
                    break;
                case INCREASE_VALUE:
                    if (channel_states[ir.get_channel()].blue.actual_step <
                        channel_states[ir.get_channel()].blue.steps)
                            channel_states[ir.get_channel()].blue.actual_step++;
                    break;
                case DECREASE_VALUE:
                    if (channel_states[ir.get_channel()].blue.actual_step >
                        -channel_states[ir.get_channel()].blue.steps) 
                            channel_states[ir.get_channel()].blue.actual_step--;
                    break;
            }
            // Event handlers triggered here.
            if (generic_handler) generic_handler(ir, channel_states[ir.get_channel()]);
            if (red_effected_handler[ir.get_channel()]  && !ir.handled && ir.red_effected() )
                red_effected_handler[ir.get_channel()](ir, channel_states[ir.get_channel()]);
            if (blue_effected_handler[ir.get_channel()] && !ir.handled && ir.blue_effected())
                blue_effected_handler[ir.get_channel()](ir, channel_states[ir.get_channel()]);
            if (red_changed_handler[ir.get_channel()] && !ir.handled
                && old_red_value != channel_states[ir.get_channel()].red.actual_step)
                    red_changed_handler[ir.get_channel()](ir, channel_states[ir.get_channel()]);
            if (blue_changed_handler[ir.get_channel()] && !ir.handled
                && old_blue_value != channel_states[ir.get_channel()].blue.actual_step)
                    blue_changed_handler[ir.get_channel()](ir, channel_states[ir.get_channel()]);
        }
    }
}

bool set_steps(uint8_t channel, uint8_t steps_red, uint8_t steps_blue) {
    if (channel > 3) return false;
    if (steps_red < 1 || steps_red > 127) return false;
    if (steps_blue < 1 || steps_blue > 127) return false;
    channel_states[channel].red.steps = steps_red;
    channel_states[channel].blue.steps = steps_blue;
    return true;
}

}; // end namespace
