///test_joystick///
#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

bool state = false;

void setup()
{
    Joystick.begin(true);
    Joystick.setButton(1, true);
}

void loop()
{
    Joystick.setButton(0, true);
    delay(1000);
    Joystick.setButton(0, false);
    delay(1000);
}