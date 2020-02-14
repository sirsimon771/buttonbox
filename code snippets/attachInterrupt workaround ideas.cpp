#include<Arduino.h>

#define attachMyInterrupt(pin, mode) attachInterrupt(digitalPinToInterrupt(pin), +[](){ myInterruptHandler(pin); }, mode)

void myInterruptHandler(uint8_t pin) {
  // use pin here
  if (pin == 4)
    digitalWrite(LED_BUILTIN, HIGH);
  else if (pin == 5)
    digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  attachMyInterrupt(4, FALLING);
  attachMyInterrupt(5, FALLING);
}

void loop() {}






void handleInterrupt(int which) {
    // do whatever you like to do with "which"
}

void interruptServiceRoutine1() {
    handleInterrupt(1);
}

void interruptServiceRoutine2() {
    handleInterrupt(2);
}

void interruptServiceRoutine3() {
    handleInterrupt(3);
}



//OR//


class RotaryEncoder {
public:
    void handleInterrupt();
};

void RotaryEncoder::handleInterrupt() {
    // do whatever you like to do
}

RotaryEncoder rotaryEncoder1;
RotaryEncoder rotaryEncoder2;
RotaryEncoder rotaryEncoder3;

void interruptServiceRoutine1() {
    rotaryEncoder1.handleInterrupt();
}

void interruptServiceRoutine2() {
    rotaryEncoder2.handleInterrupt();
}

void interruptServiceRoutine3() {
    rotaryEncoder3.handleInterrupt();
}