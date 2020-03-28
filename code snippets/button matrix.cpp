//code snippet for a button matrix with
//pushbuttons and toggleswitches

#include <Arduino.h>
#include <Joystick.h>

//creating the joystick
Joystick_ Joystick;

//number of buttons used
const int buttons = 25;

//inputting the matrix layout
//b = button | t = toggle | r = rotary-button | s = toggle2 | n = none/empty
String matrix[6][5] = { {"b", "b", "b", "b", "t"},
                        {"b", "b", "b", "b", "t"},
                        {"b", "b", "b", "b", "t"},
                        {"t", "t", "r", "r", "r"},
                        {"r", "r", "s", "s", "n"},
                        {"s", "s", "n", "n", "n"} };

int rows = 6;
int cols = 5;

int pinrows[6] = {4, 5, 6, 8, 9, 10};
int pincols[5] = {14, 15, 16, 18, 19};

//type of button, 0 = pushbutton | 1 = toggle
int type[buttons];

//number of button on joystick
int num[6][5] = {   { 0,  1,  2,  3, 12},
                    { 4,  5,  6,  7, 14},
                    { 8,  9, 10, 11, 16},
                    {18, 20, 22, 23, 24},
                    {25, 26, 27, 28, -1},
                    {30, 31, -1, -1, -1} };

//list of all currently pressed buttons for releasing (implement delay?)
int pressed[buttons] = {-1};
//state of toggles 0-4 | 0=off | 1=on
int toggleState[5] = {0};  //normal toggles
int stoggleState[2] = {0}; //type 2 toggles

//number of pressed buttons at any time
int pressedNum = 0;

void setup()
{
    Joystick.begin(true);

    readMatrixType(); //fill type[] array
    readMatrixNum();  //fill num[] array
}

void loop()
{
    //////////read matrix//////////UMSCHREIBEN///////////////
    for (int c = 0; c < cols; c++)
    {
        int pc = pincols[c];
        pinMode(pc, OUTPUT);
        digitalWrite(pc, HIGH);

        for (int r = 0; r < rows; r++)
        {
            int pr = pinrows[r];
            int brc = num[pr][pc];
            pinMode(pr, INPUT);
            bool pinStatus = digitalRead(pr);

            if (pinStatus && matrix[c][r] == "b" || "r")
            /////push buttons/////
            {
                Joystick.pressButton(brc);
                push(brc); //add brc to pushed buttons array
            }
            else if (isPressed(brc) && !pinStatus)
            /////release buttons/////
            {
                Joystick.releaseButton(brc);
                cut(brc); //remove brc from pushed buttons array
            }

            //////////press toggle switch buttons!!////////////
        }
        digitalWrite(pc, LOW);
    }

    //////////release toggles//////////
    if (pressedNum != 0)
    {
        for (int i = 0; i < pressedNum; i++)
        {
            int b = pressed[i];
            if (type[b] == 1) //if type = toggle
            {
                Joystick.releaseButton(b);
                cut(b);
            }
        }
    }
}





//////////used functions////////////////////////////////////////////////////////

bool isPressed(int val)
//checks if value is in pressed[] array
{
    for (int i = 0; i < pressedNum; i++)
    {
        if (pressed[i] == val)
        {
            return true;
        }
    }
    return false;
}

void push(int val)
//pushes value to first empty spot of pressed[] array
{
    for (int i = 0; i < buttons; i++)
    {
        if (pressed[i] == -1)
        {
            pressed[i] = val; //write passed value to first empty (-1) spot
            pressedNum++;
            return;
        }
    }
}

void cut(int val)
//removes value from pressed[] array and closes resulting gap
{
    for (int i = 0; i < pressedNum; i++) //go through pressed buttons
    {
        if (pressed[i] == val) //if given value is found, remove(-1)
        {
            //move values to close "gap"
            for (int j = i; j < pressedNum + 1; j++)
            {
                pressed[i] = pressed[i + 1];
            }
            pressedNum--;
            return;
        }
    }
}

void readMatrixType() //convert matrix to type[] array (b&r -> 0 | t&s -> 1)
{
    int counter = 0;

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (matrix[i][j] == "b" || "r")
            {
                type[counter] = 0; //pushbutton = 0
            }
            else if (matrix[i][j] == "t" || "s")
            {
                type[counter] = 1; //toggle = 1
            }
            else //if end of matrix is reached ("n"), return
            {
                return;
            }

            counter++;
        }
    }
}

void readMatrixNum() //fill num[] array from matrix[] (b>t>r>s)
{
    int current = 0; //the current

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
            else if (matrix[r][c] == "s") //toggle2 numbering
            {
                num[r][c] = current;
                current = current + 2; // toggle needs two buttons
                continue;
            }
        }
    }
}

void initializeToggles() //check for initial state of toggle (down=0 | up=1)
{
    for (int c = 0; c < cols; c++)
    {
        int pc = pincols[c];
        pinMode(pc, OUTPUT);
        digitalWrite(pc, HIGH);
        int count, count2 = 0;

        for (int r = 0; r < rows; r++)
        {
            if (matrix[c][r] == "t")
            {
                //toggle (on-off)
                if (digitalRead(pinrows[r]) == HIGH)
                {
                    toggleState[count] = 1;
                }
                count++;
            }
            else if (matrix[c][r] == "s")
            {
                //toggle2 (on-off-on)
                if (digitalRead(pinrows[r]) == HIGH)
                {
                    stoggleState[count2] = 1;
                }
            }
        }
    }
}
