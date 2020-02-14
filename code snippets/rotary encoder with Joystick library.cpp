//code snippet for one rotary encoder
//(phase 1 on pin 0, phase 2 on pin 1) with Joystick library

#include <Arduino.h>
#include <Joystick.h>

//create Joystick Object
Joystick_ Joystick;

RotaryEncoder re[5];

//phase 2 pin number
const int pinPhase2 = 20;
const int pins[5] = {3, 2, 0, 1, 7};
//rotary encoder class
class RotaryEncoder
{
private:
    int button1Num, button2Num; //button 1 = cw || button 2 = ccw
    int pinPhase1, pinPhase2;
    volatile bool lastcw, lastccw = false;

public:
    void start();       //set pinMode
    void turn();        //set joystick buttons
    void release();     //release joystick buttons (implement delay?)
    int pinNum();       //output the pin number
    int pinInt();       //output the interrupt number
    void setPin(int p); //set pin number
    void setB1(int b1); //set button 1 number
    void setB2(int b2); //set button 2 number

    //constructor, takes pin phase 1 number & 2 button numbers
    RotaryEncoder(int pin, int b1, int b2)
    {
        pinPhase1 = pin;
        button1Num = b1;
        button2Num = b2;
    }
    //constructor overload for array of objects
    RotaryEncoder()
    {
        pinPhase1 = -1;
        button1Num = -1;
        button2Num = -1;
    }
};

void RotaryEncoder::start()
{
    //set pinMode
    pinMode(pinPhase1, INPUT);
}

void RotaryEncoder::turn()
{
    //code to check for turning direction
    //and pushing the joystick buttons
    if (!lastcw && !lastccw)
    {
        if (digitalRead(pinPhase2) == HIGH)
        {
            //press button 1 (clockwise)
            Joystick.setButton(button1Num, 1);
            lastcw = true;
        }
        else if (digitalRead(pinPhase2) != HIGH)
        {
            //press button 2 (counterclockwise)
            Joystick.setButton(button2Num, 1);
            lastccw = true;
        }
    }
}

void RotaryEncoder::release()
{
    if (digitalRead(pinPhase1) == LOW && lastcw || lastccw)
    {
        //release both buttons if phase1 is low and either is pressed
        Joystick.setButton(button1Num, 0);
        Joystick.setButton(button2Num, 0);
        lastcw, lastccw == false;
    }
}

int RotaryEncoder::pinNum() { return pinPhase1; }

int RotaryEncoder::pinInt() { return digitalPinToInterrupt(pinPhase1); }

void RotaryEncoder::setPin(int p) { pinPhase1 = p; }
void RotaryEncoder::setB1(int b1) { button1Num = b1; }
void RotaryEncoder::setB2(int b2) { button2Num = b2; }

void setup()
{
    Joystick.begin(true);

    //initialize rotary encoder objects in array with values 
    for (int i = 0; i < 5; i++)
    {
        int b = 0;
        re[i].setPin(pins[i]);
        re[i].setB1(b);
        b++;
        re[i].setB2(b);
        b++;
    }

    //attach interrupts
    for (int i = 0; i < 5; i++)
    {
        attachInterrupt(re[i].pinInt, re[i].turn, RISING);
    }
}

void loop()
{
    //release joystick buttons
    for (int i = 0; i < 5; i++)
    {
        re[i].release();
    }
}