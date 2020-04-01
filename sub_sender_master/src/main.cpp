///code for the slave arduino which reads the rotary encoders///
//it is the I2C master and sends joystick button number and desired state to the main arduino
#include <Arduino.h>
#include <Wire.h>

const unsigned int pinPhase1 = 4;
const unsigned int pinPhase2 = 5;
const unsigned int rotaryHold = 20;

const unsigned int buttons[2] = {1, 2};
volatile bool lastp1 = false;
volatile bool state[2] = {false};
volatile unsigned long timer[2] = {millis()};
volatile unsigned long timerp1 = millis();

/////function prototypes/////
void send(int n, bool s);
void pinSet();
void rotaryPush();

void setup()
{
    pinSet();     //set pin modes
    Wire.begin(); //begin I2C communication as master
}

void loop()
{
    rotaryPush();
}

void pinSet() //set pin modes
{
    pinMode(pinPhase1, INPUT);
    pinMode(pinPhase2, INPUT);
}

void rotaryPush() //read rotary encoders and send data
{
    bool pin1 = digitalRead(pinPhase1);
    bool pin2 = digitalRead(pinPhase2);

    if ((pin1 != lastp1) && ((millis() - timerp1) > rotaryHold))
    {
        //phase1 changed and time check passed

        if (pin1) //phase1 changed from low to high
        {
            lastp1 = HIGH;

            if (pin2) //phase2 is high -> clockwise rotation
            {
                send(buttons[0], true);
                state[0] = true;
                timerp1 = millis();
            }
            else //phase2 is low -> counterclockwise rotation
            {
                send(buttons[1], true);
                state[1] = true;
                timerp1 = millis();
            }
        }
        else //phase1 changed from high to low
        {
            lastp1 = LOW;

            if (state[0])   //release clockwise button
            {
                send (buttons[0], false);
                state[0] = false;
            }
            if (state[1])   //release counterclockwise button
            {
                send (buttons[1], false);
                state[1] = false;
            }
        }
    }
}

//sends button num and -state to the main arduino through I2C
void send(int n, bool s)
{
    //first bit: leading 1; bit 2: empty; bits 3-6: 4-bit button number; last bit: state
    byte m = 1 << 7;      //start with leading 1
    m = m | (n << 2) | s; //combine 4-bit number with state bit

    ///send message byte///
    Wire.beginTransmission(17); //transmit to device #17
    Wire.write(m);
    Wire.endTransmission();
}