///test_toggle///
///pretty good already///
#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

const int pin = 10;
const int buttons[2] = {1, 2};

const unsigned int toggleHold = 50;
const unsigned int db = 10;

bool state = digitalRead(pin);
bool pressed[2] = {false, false};
unsigned long timers[2] = {millis()};

void setup()
{
    Serial.begin(9600);
    Joystick.begin(false);
    pinMode(pin, INPUT);
}

void loop()
{
    if (digitalRead(pin) != state && digitalRead(pin) && (abs(millis() - timers[0]) > db))
    {
        //Serial.println("1 on");
        //press ON button & start timer
        Joystick.setButton(buttons[0], true);
        state = true;
        pressed[0] = true;
        timers[0] = millis();
    }
    else if (digitalRead(pin) != state && !digitalRead(pin)  && (abs(millis() - timers[1]) > db))
    {
        //Serial.println("2 on");
        //press OFF button & start timer
        Joystick.setButton(buttons[1], true);
        state = false;
        pressed[1] = true;
        timers[1] = millis();
    }

    //release toggles
    for (int i = 0; i < 2; i++)
    {
        if (pressed[i] && (abs(millis() - timers[i]) > toggleHold))
        {
            Serial.println(pressed[i]);
            Serial.println(pressed[!i]);
            //Serial.println("off");
            Joystick.setButton(buttons[i], false);
            pressed[i] = false;
        }
    }

    Joystick.sendState();
}