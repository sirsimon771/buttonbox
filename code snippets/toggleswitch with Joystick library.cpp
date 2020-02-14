//code snippet for one toggle switch (pin 1, vs +5v) with the Joystick library
//the toggle switch actuates one button in each of its two positions
#include <Arduino.h>
#include <Joystick.h>

//create the Joystick Object
Joystick_ Joystick;
bool laston = false;
bool lastoff = false;

void setup()
{
    Joystick.begin(true);
    pinMode(0, INPUT);
}

void loop()
{

    if (digitalRead(0) == HIGH && !laston)
    {
        Joystick.setButton(0, 1); //press button 0
        laston = true;
    }
    else if (digitalRead(0) == HIGH && laston)
    {
        Joystick.setButton(0, 0); //release button 0
        laston = false;
    }
    else if (digitalRead(0) != HIGH && !lastoff)
    {
        Joystick.setButton(1, 1); //press button 1
        lastoff = true;
    }else if (digitalRead(0) != HIGH && lastoff)
    {
        Joystick.setButton(1, 0); //release button 1
        lastoff = false;
    }
}