#define IR_PIN 18
#define LED_PIN 13

byte led_status = LOW;

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(IR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), isr, FALLING);
}

void loop() {
    digitalWrite(LED_PIN, led_status);
}

void isr() {
    led_status = !led_status;
}

