#include <Arduino.h>
#include <Wire.h>
#include <Joystick.h>

//creating the joystick object
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD, 47, 0, false, false, false, false, false, false, false, false, false, false, false);

const unsigned int address = 17; //I2C address of secondary arduino
const unsigned int rotaryhold = 70;
const unsigned int rotarydeb = 20;
//pin nums -> send to secondary arduino on startup
const unsigned int rPins[5][2] = {{4, 10},
                                  {5, 16},
                                  {6, 14},
                                  {7, 15},
                                  {8, 18}};
//button nums (cw, ccw) -> send to secondary arduino on startup
const unsigned int rButtons[5][2] = {{37, 38},
                                     {39, 40},
                                     {41, 42},
                                     {43, 44},
                                     {45, 46}};

//rotary states & timers for releasing buttons
bool rstate[5][2] = {false};
unsigned long rtimer[5][2] = {millis()};

const int rows = 6; //number of matrix rows
const int cols = 6; //number of matrix columns

const int debounce = 6;     //delay in ms between button presses for debouncing
const int toggleHold = 100; //time in ms to hold toggle button pressed

//inputting the matrix layout
//b = button | t = toggle | r = rotary-button | s = o-i-o toggle | n = none/empty
const String matrix[6][6] = {{"b", "b", "b", "b", "t", "t"},
                             {"b", "b", "b", "b", "t", "t"},
                             {"b", "b", "b", "b", "t", "m"},
                             {"r", "r", "r", "r", "r", "m"},
                             {"m", "m", "m", "m", "n", "n"},
                             {"m", "m", "m", "m", "n", "n"}};

//pin numbers of all columns and rows
const int pinrows[6] = {4, 5, 6, 7, 8, 9};
const int pincols[6] = {19, 18, 15, 14, 16, 10};

//numbers of joystick-buttons
const int num[6][6] = {{0, 1, 2, 3, 12, 14},
                       {4, 5, 6, 7, 16, 18},
                       {8, 9, 10, 11, 20, 27},
                       {22, 23, 24, 25, 26, 28},
                       {29, 31, 33, 35, -1, -1},
                       {30, 32, 34, 36, -1, -1}};

//state of all matrix elements
bool state[6][6] = {false};
//toggle states for release of toggle buttons
bool tstate[6][6] = {false};

//matrix for timers (for debouncing and releasing of toggle buttons)
unsigned long timer[6][6] = {0};

//function prototypes
void setPinModes();
void initializeToggles();
void rotary(int num);
void request();

/*matrix wiring layout:

    19 |18 |15 |14 |16 |10
4   B1 |B2 |B3 |B4 |T1 |T2
5   B5 |B6 |B7 |B8 |T3 |T4
6   B9 |B10|B11|B12|T5 |M1a
7   R1 |R2 |R3 |R4 |R5 |M1b
8   M2a|M3a|M4a|M5a|
9   M2b|M3b|M4b|M5b|

matrix button numbering:
    19|18|15|14|16|10
4   0 |1 |2 |3 |12|14
5   4 |5 |6 |7 |16|18
6   8 |9 |10|11|20|27
7   22|23|24|25|26|28
8   29|31|33|35|-1|-1
9   30|32|34|36|-1|-1

rotary encoder button numbering:
 #  1  2  3  4  5     <- rotary encoder number
cw  37 39 41 43 45    <- clockwise
ccw 38 40 42 44 46    <- counterclockwise

*/

void setup()
{
  Joystick.begin(true);    //initialize joystick, AutoSendState=true
  setPinModes();           //sets all pins to the right mode
  Wire.begin(17);          //begin I2C communication as slave #17
  Wire.onReceive(rotary);  //attach receive handler
  Wire.onRequest(request); //attach request handler
  initializeToggles();     //set all toggles to initial states (press no buttons)
}

void loop()
{
  //scan matrix
  for (int c = 0; c < cols; c++)
  {
    int pc = pincols[c];   //pin of current column
    digitalWrite(pc, LOW); //pull current col pin low

    for (int r = 0; r < rows; r++)
    {
      int pr = pinrows[r];               //pin of current row
      int b = num[r][c];                 //button number of current position
      String t = matrix[r][c];           //current type ("b"|"t"|"r"|"m")
      bool pinStatus = !digitalRead(pr); //read matrix row (low means true)

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
            if (pinStatus) //toggle on if logical 1 (low)
            {
                Joystick.setButton(b, true);
                state[r][c] = true;
                tstate[r][c] = true;
            }
            else //toggle off if logical 0 (high)
            {
                Joystick.setButton(b + 1, true);
                state[r][c] = false;
                tstate[r][c] = true;
            }
        }
      }
      ////release toggle buttons////
      if (tstate[r][c] && (abs(millis() - timer[r][c]) > toggleHold))
      {
          Joystick.setButton(b, false);     //"on" button
          Joystick.setButton(b + 1, false); //"off" button
          tstate[r][c] = false;
      }
    }
    digitalWrite(pc, HIGH); //pull column pin back up
  }

  /////release rotary encoders/////
  for (int r = 0; r < 5; r++)
  {
    for (int p = 0; p < 2; p++)
    {
      if ((rstate[r][p]) && (abs(millis() - rtimer[r][p]) > rotaryhold))
      {
        Joystick.setButton(rButtons[r][p], false);
        rstate[r][p] = false;
      }
    }
  }
}

/////used functions/////
void setPinModes() //sets all pins to the right mode
{
  //matrix pins
  for (int i = 0; i < cols; i++) //set col pins to outputs
  {
    pinMode(pincols[i], OUTPUT);
    digitalWrite(pincols[i], HIGH);
  }
  for (int i = 0; i < rows; i++) //set row pins to inputs with pullups
  {
    pinMode(pinrows[i], INPUT_PULLUP);
  }
}

void initializeToggles() //set toggle states in state[][]
{
  for (int c = 0; c < cols; c++)
  {
    int pc = pincols[c];
    digitalWrite(pc, LOW); //pull current col down

    for (int r = 0; r < rows; r++)
    {
      if (matrix[r][c] == "t")
      {
        state[r][c] = digitalRead(pinrows[r]);
      }
    }
    digitalWrite(pc, HIGH); //pull current col back up
  }
}

//receive rotary updates from secondary arduino board
void rotary(int num)
{
  byte m = Wire.read();
  byte s = m & 1;           //read last bit (desired button state)
  byte n = (m >> 2) & 63;   //move 6-bit number to end, read only it
  Joystick.setButton(n, s); //set button "n" to state "s"

  for (int i = 0; i < 5; i++) //evtl umschreiben? (dictionary with button number as key and vector with [i][j] as value maybe?)
  {
    for (int j = 0; j < 2; j++)
    {
      if (rButtons[i][j] == (n))
      {
        rstate[i][j] = s;
        rtimer[i][j] = millis();
      }
    }
  }
}

//send initial data to secondary arduino on request
void request()
{
  Wire.write((byte)rotarydeb);
  /////pin numbers/////
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      Wire.write((byte)rPins[i][j]);
    }
  }
  /////button numbers/////
  for (int i = 0; i < 5; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      Wire.write((byte)rButtons[i][j]);
    }
  }
}