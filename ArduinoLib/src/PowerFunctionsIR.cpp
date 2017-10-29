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
        IRSample ir_event = dequeue();
        if (ir_event.checksum_ok()) { //TODO filter out redundant events
            if (generic_handler) generic_handler(ir_event);
            //TODO if (other_handler && !ir_event.handled) other_handler(ir_event);
        }
    }
}

}; // end namespace
