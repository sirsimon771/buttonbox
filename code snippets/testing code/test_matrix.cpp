///test_matrix///
#include <Arduino.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick;

//number of buttons used
const int buttons = 26;

const int rows = 6;   //number of matrix rows
const int cols = 5;   //number of matrix columns

const int debounce = 6;  //delay in ms between button presses for debouncing
const int toggleHold = 200;  //time in ms to hold toggle button pressed

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

void setup()
{
    Joystick.begin(true); //initialize joystick, AutoSendState=true
    setPinModes();        //sets all pins to the right mode
    readMatrixNum();      //fill num[] array
    initializeToggles();  //set all toggles to initial states (press no buttons)
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