//code snippet for one button (pin 0, vs +5v) with the Joystick library
#include <Arduino.h>
#include <Joystick.h>

//create the Joytick Object
Joystick_ Joystick;
bool last = false;

void setup()
{
    Joystick.begin(true);
    pinMode(0, INPUT);
}

void loop()
{

    if (digitalRead(0) == HIGH && !last)
    {
        Joystick.setButton(0, 1); //button pressed
        last = true;
    }
    else if (digitalRead(0) != HIGH && last)
    {
        Joystick.setButton(0, 0); //button released
        last = false;
    }
}