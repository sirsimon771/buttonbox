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

    14 |15 |16 |18 |19
4   B1 |B2 |B3 |B4 |T1
5   B5 |B6 |B7 |B8 |T2
6   B9 |B10|B11|B12|T3
8   T4 |T5 |R1 |R2 |R3
9   R4 |R5 |S1a|S1b|S2a
10  S2b|

interrupt pins for rotary encoders phase 1
3(INT0)|2(INT1)|0(INT2)|1(INT3)|7(INT6)|20
R1p1   |R2p1   |R3p1   |R4p1   |R5p1   |R*p2

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
