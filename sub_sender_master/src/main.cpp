///code for the slave arduino which reads the rotary encoders///
//it is the I2C master and sends joystick button number and desired state to the main arduino
//at startup it requests some data from the slave
#include <Arduino.h>
#include <Wire.h>

unsigned int debounce = 20;
unsigned int pins[5][2] =  {{4, 10},
                            {5, 16},
                            {6, 14},
                            {7, 15},
                            {8, 18}};

unsigned int buttons[5][2] =   {{37, 38},
                                {39, 40},
                                {41, 42},
                                {43, 44},
                                {45, 46}};

volatile bool lastp1[5] = {true};
volatile unsigned long timer[5][2] = {millis()};
volatile unsigned long timerp1 = millis();

/////function prototypes/////
void send(int n, bool s);
void pinSet();
void rotaryPush();
void request();

void setup()
{
    pinSet();     //set pin modes
    Wire.begin(); //begin I2C communication as master
    //Serial.begin(9600);
    //request();
}

void loop()
{
    rotaryPush();
}

void pinSet() //set pin modes
{
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            pinMode(pins[i][j], INPUT_PULLUP);
        }
    }
}

void rotaryPush() //read rotary encoders and send data
{
    for (int r = 0; r < 5; r++)
    {
        bool pin1 = digitalRead(pins[r][0]);
        bool pin2 = digitalRead(pins[r][1]);

        if ((pin1 != lastp1[r]) && ((millis() - timerp1) > debounce))
        {
            //phase1 changed and time check passed
            lastp1[r] = pin1;

            if (!pin1) //phase1 changed from high to low
            {

                if (pin2) //phase2 is high -> clockwise rotation
                {
                    send(buttons[r][0], true);
                    //Serial.print("\nclockwise, button:");
                    //Serial.print(buttons[r][0]);
                    timerp1 = millis();
                }
                else //phase2 is low -> counterclockwise rotation
                {
                    send(buttons[r][1], true);
                    //Serial.print("\ncounterclockwise, button:");
                    //Serial.print(buttons[r][1]);
                    timerp1 = millis();
                }
            }
            
        }
    }
}


//sends button num and -state to the main arduino through I2C
void send(int n, bool s)
{
    //first bit: leading 1; bit 2: empty; bits 3-6: 4-bit button number; last bit: state
    byte m = 1 << 7;      //start with leading 1
    m = m | (n << 2) | s; //combine 6-bit number with state bit

    ///send message byte///
    Wire.beginTransmission(17); //transmit to device #17
    Wire.write(m);
    Wire.endTransmission();
}

//request & save initial data from I2C slave
void request()
{
    Wire.requestFrom(17, 21);

    int i = 0;
    while (Wire.available())
    {
        if(i < 1)       //first byte
        {
            debounce = Wire.read();
        }
        else if (i < 11) //bytes containing pin numbers
        {
            pins[(int)ceil(i / 2)][(i + 1) % 2] = Wire.read();
        }
        else            //bytes containing button numbers
        {
            buttons[(int)ceil((i - 10) / 2)][(i + 1) % 2] = Wire.read();
        }

        i++;
    }
}