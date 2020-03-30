#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

const int pinPhase2 = 21;             //phase 2 pin number
const int rpins[5] = {3, 2, 0, 1, 7};       //phase 1 pins

//button nums (cw, ccw)
const int rButtons[2][5] = {  {37, 39, 41, 43, 45},
                              {38, 40, 42, 44, 46}  };

volatile bool lastcw[5] = {false};    //state of clockwise button
volatile bool lastccw[5] = {false};   //state of counterclockwise button

//number of buttons used
const int buttons = 47;

const int rows = 6;   //number of matrix rows
const int cols = 6;   //number of matrix columns

const int debounce = 6;  //delay in ms between button presses for debouncing
const int toggleHold = 200;  //time in ms to hold toggle button pressed
const int rotaryHold = 70;  //time in ms to hold rotary button pressed

//inputting the matrix layout
//b = button | t = toggle | r = rotary-button | s = o-i-o toggle | n = none/empty
const String matrix[6][6] = {   {"b", "b", "b", "b", "t", "t"},
                                {"b", "b", "b", "b", "t", "t"},
                                {"b", "b", "b", "b", "t", "m"},
                                {"r", "r", "r", "r", "r", "m"},
                                {"m", "m", "m", "m", "n", "n"},
                                {"m", "m", "m", "m", "n", "n"} };

//pin numbers of all columns and rows
const int pinrows[6] = {4, 5, 6, 8, 9, 10};
const int pincols[6] = {14, 15, 16, 18, 19, 20};

//numbers of joystick-buttons
const int num[6][6] = { { 0,  1,  2,  3, 12, 14},
                        { 4,  5,  6,  7, 16, 18},
                        { 8,  9, 10, 11, 20, 27},
                        {22, 23, 24, 25, 26, 28},
                        {29, 31, 33, 35, -1, -1},
                        {30, 32, 34, 36, -1, -1}  };    

//state of all matrix elements
bool state[6][6] = {false};

//matrix for timers (for debouncing and releasing of toggle buttons)
unsigned long timer[6][6] = {0};
//timers for releasing rotary encoder buttons (two each)
volatile unsigned long rtimer[2][5] = {millis()};

//function prototypes
void setPinModes();
void initializeToggles();
void rotaryRelease();

//interupt handlers prototypes
void interruptRotary1();
void interruptRotary2();
void interruptRotary3();
void interruptRotary4();
void interruptRotary5();

/*matrix wiring layout:

    14 |15 |16 |18 |19 |20
4   B1 |B2 |B3 |B4 |T1 |T2
5   B5 |B6 |B7 |B8 |T3 |T4
6   B9 |B10|B11|B12|T5 |M1a
8   R1 |R2 |R3 |R4 |R5 |M1b
9   M2a|M3a|M4a|M5a|
10  M2b|M3b|M4b|M5b|

interrupt pins for rotary encoders phase 1
3(INT0)|2(INT1)|0(INT2)|1(INT3)|7(INT6)|21
R1p1   |R2p1   |R3p1   |R4p1   |R5p1   |R*p2

matrix button numbering:
    14|15|16|18|19|20
4   0 |1 |2 |3 |12|14
5   4 |5 |6 |7 |16|18
6   8 |9 |10|11|20|27
8   22|23|24|25|26|28
9   29|31|33|35|-1|-1
10  30|32|34|36|-1|-1

*/

void setup()
{
  Joystick.begin(true); //initialize joystick, AutoSendState=true
  setPinModes();        //sets all pins to the right mode
  initializeToggles();  //set all toggles to initial states (press no buttons)

  //attach interrupts //make seperate functions!!!!!
  attachInterrupt(3, interruptRotary1, RISING);
  attachInterrupt(2, interruptRotary2, RISING);
  attachInterrupt(0, interruptRotary3, RISING);
  attachInterrupt(1, interruptRotary4, RISING);
  attachInterrupt(7, interruptRotary5, RISING);
}

void loop()
{
  //scan matrix
  for (int c = 0; c < cols; c++)
  {
    int pc = pincols[c];    //pin of current column
    digitalWrite(pc, HIGH); //pull current col pin high

    for (int r = 0; r < rows; r++)
    {
      int pr = pinrows[r];              //pin of current row
      int b = num[r][c];                //button number of current position
      String t = matrix[r][c];          //current type ("b"|"t"|"r"|"m")
      bool pinStatus = digitalRead(pr); //read matrix row

      //read state != saved state -> do something
      if (pinStatus != state[r][c] && abs(millis() - timer[r][c]) > debounce)
      {
        timer[r][c] = millis(); //reset timer

        ////pushbuttons////
        if (t == "b" || "r" || "m")
        {
          Joystick.setButton(b, pinStatus); //push or release button
          state[r][c] = pinStatus;          //invert state variable
        }

        ////toggle switches////press b for "on" & b+1 for "off"////
        if (t == "t")
        {
          if (pinStatus) //toggle on
          {
            Joystick.setButton(b, true);
            state[r][c] = true;
          }
          else //toggle off
          {
            Joystick.setButton(b + 1, true);
            state[r][c] = false;
          }
        }
        //// o-i-o toggles////press b for "up" | b+1 for "down" | b
        else if (t == "sa" || "sb")
        {
          if (pinStatus) //toggle on
          {
            Joystick.setButton(b, true);
            state[r][c] = true;
          }
          else //toggle off
          {
            int boff = 2; //button offset for the "off" button if t == "sa"
            if (t == "sb")
            {
              boff = 1; //button offset for second pin
            }
            Joystick.setButton(b + boff, true); //add button offset for "off"
            state[r][c] = false;
          }
        }
      }

      ////release toggle buttons////
      if ((t == "t" || "sa" || "sb") && (abs(millis() - timer[r][c]) > toggleHold))
      {
        Joystick.setButton(b, false);     //"on" button
        Joystick.setButton(b + 1, false); //"off" button
      }
    }
    digitalWrite(pc, LOW);  //pull column pin back down
  }
  /////release rotary encoder buttons/////
  rotaryRelease();
}



/////used functions/////
void setPinModes()  //sets all pins to the right mode
{
  //matrix pins
  for (int i = 0; i < cols; i++)  //set col pins to outputs
  {
    pinMode(pincols[i], OUTPUT);
  }
  for (int i = 0; i < rows; i++)  //set row pins to inputs
  {
    pinMode(pinrows[i], INPUT);
  }

  //rotary encoder phase2 pin
  pinMode(pinPhase2, INPUT);
}

void initializeToggles()  //set toggle states in state[][]
{
  for (int c = 0; c < cols; c++)
  {
    int pc = pincols[c];
    digitalWrite(pc, HIGH); //pull current col to high

    for (int r = 0; r < rows; r++)
    {
      if(matrix[r][c] == "t" || "sa" || "sb")
      {
        state[r][c] = digitalRead(pinrows[r]);
      }
    }
    digitalWrite(pc, LOW);  //pull current col back to low
  }
}

void rotaryRelease()
{
  for (int i = 0; i < 5; i++)
  {
    //release buttons if phase1 is low, last is true & timer is > holding time
    if (digitalRead(rpins[i]) == LOW && ( lastcw[i] || lastccw[i]) )
    {
      if (abs(millis() - timer[0][i]) > rotaryHold)
      {
        Joystick.setButton(rButtons[0][i], false);
        lastcw[i] = false;
      }
      if (abs(millis() - timer[1][i]) > rotaryHold)
      {
        Joystick.setButton(rButtons[1][i], false);
        lastccw[i] = false;
      }

      Joystick.setButton(rButtons[0][i], 0);
      Joystick.setButton(rButtons[1][i], 0);
      lastcw[i], lastccw[i] = false;
    }
  }
}





/////interrupt handlers/////
void interruptRotary1()
{
  //check for turning direction
  //and push repective joystick buttons for rotary 1
  if (digitalRead(pinPhase2) && (abs(millis() - timer[0][0]) > debounce))
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][0], 1);
    lastcw[0] = true;
    timer[0][0] = millis();
  }
  else if (abs(millis() - timer[1][0]) > debounce)
  {
    //press button 2 (counterclockwise)
    Joystick.setButton(rButtons[1][0], 1);
    lastccw[0] = true;
    timer[0][1] = millis();
  }
}
void interruptRotary2()
{
  //check for turning direction
  //and push repective joystick buttons for rotary 2
  if (digitalRead(pinPhase2) && (abs(millis() - timer[0][1]) > debounce))
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][1], 1);
    lastcw[1] = true;
  }
  else if (abs(millis() - timer[1][1]) > debounce)
  {
    //press button 2 (counterclockwise)
    Joystick.setButton(rButtons[1][1], 1);
    lastccw[1] = true;
  }
}
void interruptRotary3()
{
  //check for turning direction
  //and push repective joystick buttons for rotary 3
  if (digitalRead(pinPhase2) && (abs(millis() - timer[0][2]) > debounce))
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][2], 1);
    lastcw[2] = true;
  }
  else if (abs(millis() - timer[1][2]) > debounce)
  {
    //press button 2 (counterclockwise)
    Joystick.setButton(rButtons[1][2], 1);
    lastccw[2] = true;
  }
}
void interruptRotary4()
{
  //check for turning direction
  //and push repective joystick buttons for rotary 4
  if (digitalRead(pinPhase2) && (abs(millis() - timer[0][3]) > debounce))
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][3], 1);
    lastcw[3] = true;
  }
  else if (abs(millis() - timer[1][3]) > debounce)
  {
    //press button 2 (counterclockwise)
    Joystick.setButton(rButtons[1][3], 1);
    lastccw[3] = true;
  }
}
void interruptRotary5()
{
  //check for turning direction
  //and push repective joystick buttons for rotary 5
  if (digitalRead(pinPhase2) && (abs(millis() - timer[0][4]) > debounce))
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][4], 1);
    lastcw[4] = true;
  }
  else if (abs(millis() - timer[1][4]) > debounce)
  {
    //press button 2 (counterclockwise)
    Joystick.setButton(rButtons[1][4], 1);
    lastccw[4] = true;
  }
}