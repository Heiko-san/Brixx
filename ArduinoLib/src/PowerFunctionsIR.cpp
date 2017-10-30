/*
 * PowerFunctionsIR.cpp - Library to read LEGO Power Fundtions remote controls
 * Copyright© 2017 by Heiko Finzel
 * The Brixx Library and all other content of this project is distributed under the terms and conditions
 * of the GNU GENERAL PUBLIC LICENSE Version 3.
 * Note: Although this project aims to connect LEGO Power Functions hardware to Arduino without
 * damaging anything else then just a few extension cables, I'll take no responsibility if it happens
 * afterall. Your warranty for your LEGO Power Functions items may void using this project.
 */
#include "PowerFunctionsIR.h"

namespace PowerFunctionsIR {

// sampling buffer
IRSample sample_value;
// remember last sample interrupt time
volatile unsigned long sample_micros_last = 0;
// position of the bit to sample next
volatile int8_t sample_position = 15;
// our interrupt address (derived from interrupt pin in init())
uint8_t interrupt_address;
// saved states for all 4 channels
ChannelState channel_states[4];

void sample_isr( void ) {
    unsigned long micros_now = micros();
    uint16_t micros_diff = micros_now - sample_micros_last;
    sample_micros_last = micros_now;
    if (micros_diff < START_STOP_MAX && micros_diff > LOW_MIN) {
        if (micros_diff > START_STOP_MIN) {
            // restart sampling on early start-stop-signal (most certainly a new, interfering signal)
            sample_value.raw = 0;
            sample_position = 15;
        } else if (micros_diff > HIGH_MIN) {
            // high
            sample_value.raw |= 1 << sample_position;
            sample_position -= 1;
        } else {
            // low (add 0 << sample_position => no change)
            sample_position -= 1;
        }
        if (sample_position < 0) {
            enqueue(sample_value);
            attachInterrupt(interrupt_address, idle_isr, FALLING);
        }
    } else {
        // reset on signal error
        attachInterrupt(interrupt_address, idle_isr, FALLING);
    }
}

void idle_isr( void ) {
    unsigned long micros_now = micros();
    uint16_t micros_diff = micros_now - sample_micros_last;
    sample_micros_last = micros_now;
    if (micros_diff < START_STOP_MAX && micros_diff > START_STOP_MIN) {
        // start sampling on start-stop-signal
        sample_value.raw = 0;
        sample_position = 15;
        attachInterrupt(interrupt_address, sample_isr, FALLING);
    }
}

void init( void ) {
    sample_value.raw = 0;
    for (int i = 0; i < 4; i++) {
        channel_states[i].red.step = DEFAULT_STEP;
        channel_states[i].blue.step = DEFAULT_STEP;
    }
    pinMode(IR_SAMPLE_INTERRUPT_PIN, INPUT);
    interrupt_address = digitalPinToInterrupt(IR_SAMPLE_INTERRUPT_PIN);
    attachInterrupt(interrupt_address, idle_isr, FALLING);
}

EventHandler generic_handler;
// the queue's first and last element
Event* head;
Event* tail;

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
        if we remove the last element of the queue this is a critical action
        to ensure queue doesn't get corrupted if interrupt adds an element at the
        wrong moment we lock interrupts until queue is in a consistent state again
    */
    if (head == tail) noInterrupts();
    head = head->next;
    if (!head) tail = 0;
    // unlock ...
    interrupts();
    IRSample sample_to_return = p_event->ir_event;
    free(p_event);
    return sample_to_return;
}

void update( void ) {
    while ( head ) {
        IRSample ir = dequeue();
        /*
            we can't rely on toggle bit only for redundancy check, since there are edge cases where it doesn't work
            * first signal received could be 0 or 1
            * when going from full forward to full backward on standard rc the stop signal will 
              be missed and therefore both signals have same toggle
            * we could use standard rc and pwm rc on the same channel and differentiate them by mode
            we still have an edge case: if first signal is on toggle 0 with data 0 and extended mode (0)
            we will miss it, but in reality extended mode isn't in use
        */
        if (ir.checksum_ok() && channel_states[ir.channel].previous != ir.get_state_signature()) {
            channel_states[ir.channel].previous = ir.get_state_signature();
            if (generic_handler) generic_handler(ir, channel_states[ir.channel]);
            //TODO if (other_handler && !ir.handled) other_handler(ir);
        }
    }
}

}; // end namespace
