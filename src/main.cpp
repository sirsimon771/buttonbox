#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

const int pinPhase2 = 21;             //phase 2 pin number
const int rpins[5] = {3, 2, 0, 1, 7};       //phase 1 pins

//button nums (cw, ccw)
int rButtons[2][5] = {-1};

volatile bool lastcw[5] = {false};    //state of clockwise button
volatile bool lastccw[5] = {false};   //state of counterclockwise button

//number of buttons used
int buttons;

const int rows = 6;   //number of matrix rows
const int cols = 6;   //number of matrix columns

const int debounce = 6;  //delay in ms between button presses for debouncing
const int toggleHold = 200;  //time in ms to hold toggle button pressed
const int rotaryHold = 70;  //time in ms to hold rotary button pressed

//inputting the matrix layout
//b = button | t = toggle | r = rotary-button | s = o-i-o toggle | n = none/empty
String matrix[rows][cols] = {   {"b", "b", "b", "b", "t", "t"},
                                {"b", "b", "b", "b", "t", "t"},
                                {"b", "b", "b", "b", "t", "r"},
                                {"r", "r", "r", "r", "m", "m"},
                                {"m", "m", "m", "m", "m", "m"},
                                {"m", "m", "n", "n", "n", "n"} };

int pinrows[rows] = {4, 5, 6, 8, 9, 10};
int pincols[cols] = {14, 15, 16, 18, 19, 20};

//number of buttons on joystick
int num[rows][cols] = {-1};

//state of all matrix elements
bool state[rows][cols] = {0};

//state of toggles | 0=off 1=on
int toggleState[5] = {0};
int stoggleState[2] = {0};

//matrix for timers (for debouncing and releasing of toggle buttons)
unsigned long timer[rows][cols] = {0};
//timers for releasing rotary encoder buttons (two each)
volatile unsigned long rtimer[2][5] = {millis()};

//function prototypes
void setPinModes();
void readMatrixNum();
void initializeToggles();
void rotaryRelease();

//interupt handlers prototypes
void interruptRotary1();
void interruptRotary2();
void interruptRotary3();
void interruptRotary4();
void interruptRotary5();

/*Matrix wiring layout:

     14 |15 |16 |18 |19 |20
4  | B1 |B2 |B3 |B4 |T1 |T2
5  | B5 |B6 |B7 |B8 |T3 |T4
6  | B9 |B10|B11|B12|T5 |R1
8  | R2 |R3 |R4 |R5 |M1a|M1b
9  | M2a|M2b|M3a|M3b|M4a|M4b
10 | M5a|M5b|

interrupt pins for rotary encoders phase 1
3(INT0)|2(INT1)|0(INT2)|1(INT3)|7(INT6)|21
R1p1   |R2p1   |R3p1   |R4p1   |R5p1   |R*p2

    14|15|16|18|19|20
4   0 |1 |2 |3 |12|14
5   4 |5 |6 |7 |16|18
6   8 |9 |10|11|20|22
8   23|24|25|26|27|28
9   29|30|31|32|33|34
10  35|36|

*/

void setup()
{
  Joystick.begin(true); //initialize joystick, AutoSendState=true
  setPinModes();        //sets all pins to the right mode
  readMatrixNum();      //fill num[] array
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

void readMatrixNum()  //fills the num[][] matrix with button numbers
{
  double current = 0; //the current button-counter

  for (int c = 0; c < cols; c++)
  {
    for (int r = 0; r < rows; r++)
    {
      if (matrix[r][c] == "b") //button numbering
      {
        num[r][c] = current;
        current++;
      }
      else if (matrix[r][c] == "t") //toggle numbering
      {
        num[r][c] = current;
        current = current + 2; //toggle needs two buttons
      }
      else if (matrix[r][c] == "r") //rotary pushbutton numbering
      {
        num[r][c] = current;
        current++;
      }
      else if (matrix[r][c] == "m") //momentary toggle numbering
      {
        num[r][c] = current;
        current++;
      }
      else if (matrix[r][c] == "sa" || "sb")  //3-position toggles numbering
      {
        num[r][c] = floor(current);
        current = current + 1.5;
      }
    }
  }
  //go through rotary encoder buttons (2 each, clockwise & counterclockwise)
  for (int r = 0; r < 2; r++)
  {
    for (int c = 0; c < 5; c++)
    {
      rButtons[r][c] = current;
      current++;
    }
  }
  buttons = current;  //number of buttons
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