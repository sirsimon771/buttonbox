///test rotary///
#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

const unsigned int pinPhase1 = 2;   //needs to be an interrupt pin
const unsigned int pinPhase2 = 3;   //normal input pin
const unsigned int rotaryHold = 70; //time to hold button pressed
const unsigned int debounce = 5;   //debounce time

const unsigned int buttons[2] = {1, 2}; //clockwise: 0 || counterclockwise: 1
volatile bool lastcw = false;
volatile bool lastccw = false;
volatile unsigned long timer[2] = {millis()};

//function prototypes
void interruptRotary();
void rotaryRelease();

void setup()
{
    Joystick.begin(true);
    pinMode(pinPhase1, INPUT);
    pinMode(pinPhase2, INPUT);

    //attach interrupt to phase 1 pin
    attachInterrupt(digitalPinToInterrupt(pinPhase1), interruptRotary, RISING);
}

void loop()
{
    rotaryRelease();
}

void rotaryRelease()
{
    if ((digitalRead(pinPhase1) == LOW) && (lastcw || lastccw))
    {
        if (abs(millis() - timer[0]) > rotaryHold)
        {
            Joystick.setButton(buttons[0], false);
            lastcw = false;
        }
        if (abs(millis() - timer[1]) > rotaryHold)
        {
            Joystick.setButton(buttons[1], false);
            lastccw = false;
        }
    }
}

/////interrupt handler/////
void interruptRotary()
{
    //check for turning direction and push respective joystick buttons
    if (digitalRead(pinPhase2) && (abs(millis() - timer[0]) > debounce))
    {
        //push clockwise button (button 0)
        Joystick.setButton(buttons[0], true);
        lastcw = true;
        timer[0] = millis();
    }
    else if ((abs(millis() - timer[1]) > debounce))
    {
        //push counterclockwise button (button 1)
        Joystick.setButton(buttons[1], true);
        lastccw = true;
        timer[1] = millis();
    }
}