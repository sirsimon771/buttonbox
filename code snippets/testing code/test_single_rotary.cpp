///test single rotary///
///still room for improvement, occasional misfires///
#include <Arduino.h>
#include <Joystick.h>

Joystick_ Joystick;
const unsigned int pin1 = 3; //needs to be an interrupt pin
const unsigned int pin2 = 9; //normal input pin
const unsigned int debounce = 100;
volatile unsigned long timer1 = millis();
volatile unsigned long timer2 = millis();
volatile unsigned long timer = millis();

//function prototypes
void interrupt();

void setup()
{
    Serial.begin(9600);
    pinMode(pin1, INPUT);
    pinMode(pin2, INPUT);
    Joystick.begin(false);
    //attach interrupt to phase 1 pin
    attachInterrupt(digitalPinToInterrupt(pin1), interrupt, RISING);
}

void loop()
{
    if (abs(millis() - timer1) > 100)
    {
        Joystick.setButton(1, false);
    }
    if (abs(millis() - timer2) > 100)
    {
        Joystick.setButton(6, false);
    }
    Joystick.sendState();
}

/////interrupt handler/////
void interrupt()
{
    if (abs(millis() - timer) > debounce)
    {
        if (digitalRead(pin2))
        {
            Serial.println("clockwise (pin2 high)");
            timer = millis();
            Joystick.setButton(1, true);
            timer1 = millis();
        }
        else
        {
            Serial.println("counterclockwise (pin2 low)");
            timer = millis();
            Joystick.setButton(6, true);
            timer2 = millis();
        }
    }
    Joystick.sendState();
}
