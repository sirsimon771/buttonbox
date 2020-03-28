///test_button///
///pretty good///
#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

const int pin = 10;
const int button = 7;
unsigned long timer = millis();
const unsigned int debounce = 10;

bool state = false;

void setup()
{
    Joystick.begin(true);
    pinMode(pin, INPUT);
}

void loop()
{

    bool status = digitalRead(pin);
    
    if((status != state) && abs(millis() - timer) > debounce)
    {
        Joystick.setButton(button, status);
        state = status;
        timer = millis();

    }
}