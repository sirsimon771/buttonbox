#include <Arduino.h>
#include <Joystick.h>

int i = 0;

/*  Joystick Library functions

Joystick.begin(bool initAutoSendState)
Joystick.end()
Joystick.sendState()

Joystick.pressButton(uint8_t button)
Joystick.releaseButton(uint8_t button)
Joystick.setButton(uint8_t button, uint8_t value)

*/

/*Matrix wiring layout:

    6   7   8   9   10
0   B1  B2  B3  B4  T1
1   B5  B6  B7  B8  T2
2   B9  B10 B11 B12 T3
3   T4  T5  R1  R2  R3
4   R4  R5  S1a  S1b S2a
5   S2b

14      15      16      18      19      20
R1p1    R2p1    R3p1    R4p1    R5p1    R*p2

*/

void setup() {

  Serial.begin(9600);

}

void loop() {

  delay(750);
  Serial.print("\r\nHello World! - ");
  Serial.print(i);
  i++;
}
