#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

const int pinPhase2 = 20;             //phase 2 pin number
const int rpins[5] = {3, 2, 0, 1, 7};  //phase 1 pins

//button nums (cw, ccw)
const int rButtons[2][5] = {  {33, 35, 37, 39, 41},
                              {34, 36, 38, 40, 42} };

volatile bool lastcw[5] = {false};    //state of clockwise button
volatile bool lastccw[5] = {false};   //state of counterclockwise button

//number of buttons used
const int buttons = 26;

const int rows = 6;   //number of matrix rows
const int cols = 5;   //number of matrix columns

const int debounce = 6;  //delay in ms between button presses for debouncing
const int toggleHold = 200;  //time in ms to hold toggle button pressed
const int rotaryHold = 20;  //time in ms to hold rotary button pressed

//inputting the matrix layout
//b = button | t = toggle | r = rotary-button | s = o-i-o toggle | n = none/empty
String matrix[rows][cols] = {   {"b", "b", "b", "b", "t"},
                                {"b", "b", "b", "b", "t"},
                                {"b", "b", "b", "b", "t"},
                                {"t", "t", "r", "r", "r"},
                                {"r", "r", "sa", "sb", "n"},
                                {"sa", "sb", "n", "n", "n"} };

int pinrows[rows] = {4, 5, 6, 8, 9, 10};
int pincols[cols] = {14, 15, 16, 18, 19};

//number of buttons on joystick
int num[rows][cols] = {-1};

//state of all matrix elements
bool state[rows][cols] = {0};

//state of toggles | 0=off 1=on
int toggleState[5] = {0};
int stoggleState[2] = {0};

//matrix for timers (for debouncing and releasing of toggle buttons)
int timer[rows][cols] = {0};

//function prototypes
void setPinModes();
void readMatrixNum();
void initializeToggles();

//interupt handlers prototypes
void interruptRotary1();
void interruptRotary2();
void interruptRotary3();
void interruptRotary4();
void interruptRotary5();

/*Matrix wiring layout:

    14 |15 |16 |18 |19
4   B1 |B2 |B3 |B4 |T1
5   B5 |B6 |B7 |B8 |T2
6   B9 |B10|B11|B12|T3
8   T4 |T5 |R1 |R2 |R3
9   R4 |R5 |S1a|S1b|
10  S2a|S2b|

interrupt pins for rotary encoders phase 1
3(INT0)|2(INT1)|0(INT2)|1(INT3)|7(INT6)|20
R1p1   |R2p1   |R3p1   |R4p1   |R5p1   |R*p2

    14|15|16|18|19
4   0 |1 |2 |3 |12
5   4 |5 |6 |7 |14
6   8 |9 |10|11|16
8   18|20|22|23|24
9   25|26|27|28|
10  30|31|

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
    digitalWrite(pc, HIGH); //set current col to high

    for (int r = 0; r < rows; r++)
    {
      int pr = pinrows[r];              //pin of current row
      int b = num[r][c];                //button number of current position
      String t = matrix[r][c];          //current type ("b"|"t"|"r"|"sa"/"sb")
      bool pinStatus = digitalRead(pr); //read matrix row

      //read state != saved state -> do something
      if (pinStatus != state[r][c] && abs(millis() - timer[r][c]) > debounce)
      {
        timer[r][c] = millis(); //reset timer

        ////pushbuttons////
        if (t == "b" || "r")
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
      if (abs(millis() - timer[r][c]) > toggleHold)
      {
        Joystick.setButton(b, false);     //"on" button
        Joystick.setButton(b + 1, false); //"off" button
      }
    }
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

  //rotary encoder pins (just the phase 2 pin)
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
        continue;
      }
      else if (matrix[r][c] == "t") //toggle numbering
      {
        num[r][c] = current;
        current = current + 2; //toggle needs two buttons
        continue;
      }
      else if (matrix[r][c] == "r") //rotary pushbutton numbering
      {
        num[r][c] = current;
        current++;
        continue;
      }
      else if (matrix[r][c] == "sa" || "sb") //toggle2 numbering
      {
        num[r][c] = floor(current);
        current = current + 1.5; // toggle2 needs 3 buttons
        continue;
      }
    }
  }
}

void initializeToggles()  //set toggle states in state[][]
{
  for (int c = 0; c < cols; c++)
  {
    digitalWrite(pincols[c], HIGH); //set current col to high

    for (int r = 0; r < rows; r++)
    {
      if(matrix[r][c] == "t" || "sa" || "sb")
      {
        state[r][c] = digitalRead(pinrows[r]);
      }
    }
  }
}

void rotaryRelease()
{
  for (int i = 0; i < 5; i++)
  {
    //release both buttons if phase 1 is low and either is pressed
    if (digitalRead(rpins[i]) == LOW && ( lastcw[i] || lastccw[i]) )
    {
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
  if (digitalRead(pinPhase2) == HIGH)
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][0], 1);
    lastcw[0] = true;
  }
  else if (digitalRead(pinPhase2) == LOW)
  {
    //press button 2 (counterclockwise)
    Joystick.setButton(rButtons[1][0], 1);
    lastccw[0] = true;
  }
}
void interruptRotary2()
{
  //check for turning direction
  //and push repective joystick buttons for rotary 2
  if (digitalRead(pinPhase2) == HIGH)
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][1], 1);
    lastcw[1] = true;
  }
  else if (digitalRead(pinPhase2) == LOW)
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
  if (digitalRead(pinPhase2) == HIGH)
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][2], 1);
    lastcw[2] = true;
  }
  else if (digitalRead(pinPhase2) == LOW)
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
  if (digitalRead(pinPhase2) == HIGH)
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][3], 1);
    lastcw[3] = true;
  }
  else if (digitalRead(pinPhase2) == LOW)
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
  if (digitalRead(pinPhase2) == HIGH)
  {
    //press button 1 (clockwise)
    Joystick.setButton(rButtons[0][4], 1);
    lastcw[4] = true;
  }
  else if (digitalRead(pinPhase2) == LOW)
  {
    //press button 2 (counterclockwise)
    Joystick.setButton(rButtons[1][4], 1);
    lastccw[4] = true;
  }
}