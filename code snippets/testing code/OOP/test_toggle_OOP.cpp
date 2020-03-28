///test toggle OOP///
///much worse than other implementation///
#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

//toggle class
class Toggle
{
private:
    bool logging = false;

public:
    const unsigned int toggleHold = 50;
    const unsigned int debounce = 10;

    int pin;
    int buttons[2];
    bool state;
    unsigned long timers[2];
    bool pressed[2] = {false};

    //constructor
    Toggle(int p, int b1, int b2)
    {
        pin = p;
        buttons[0] = b1;
        buttons[1] = b2;
        state = digitalRead(pin);
        timers[0] = millis();
        timers[1] = millis();
    }

    void setPins()
    {
        pinMode(pin, INPUT);
    }

    void toggleOn(bool pos)
    {
        Joystick.setButton(buttons[!pos], true);
        state = pos;
        pressed[!pos] = true;
        timers[!pos] = millis();

        if (logging)
        {
            Serial.print("\nstate: ");
            Serial.print(state);
        }
    }

    void toggleOff(int i)
    {
        Joystick.setButton(buttons[i], false);
        pressed[i] = false;
    }
};

Toggle toggle1(10, 1, 2);

void setup()
{
    Serial.begin(9600);
    Joystick.begin(false);
    toggle1.setPins();
}

void loop()
{
    if (digitalRead(toggle1.pin) != toggle1.state)
    {
        if (abs(millis() - toggle1.timers[!digitalRead(toggle1.pin)]) > toggle1.debounce)
        {
            toggle1.toggleOn(digitalRead(toggle1.pin));
        }
    }

    //release toggles
    for (int i = 0; i < 2; i++)
    {
        if (toggle1.pressed[i] && (abs(millis() - toggle1.timers[i]) > toggle1.toggleHold))
        {
            toggle1.toggleOff(i);
        }
    }

    Joystick.sendState();
}