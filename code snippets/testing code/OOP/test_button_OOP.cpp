///test button OOP///
///pretty good///
#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

//button class
class Button {

    private:


    public:
    int pin;
    int button;
    unsigned long timer;
    const unsigned int debounce = 10;
    bool state = false;

    //constructor
    Button(int p, int b)
    {
        pin = p;
        button = b;
        state = false;
        timer = millis();
    }


    bool update(bool status)
    {
       Joystick.setButton(button, status);
       state = status;
       timer = millis();
       return status;
    }

    void setPins()
    {
        pinMode(pin, INPUT);
    }

};

Button button1(10, 6);

void setup()
{
    Joystick.begin(false);
    button1.setPins();
}

void loop()
{
    bool status = digitalRead(button1.pin);

    if((status != button1.state) && (abs(millis() - button1.timer) > button1.debounce))
    {
        button1.update(status);
    }

    Joystick.sendState();
}