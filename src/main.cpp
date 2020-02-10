#include <Arduino.h>
#include <Joystick.h>

int i = 0;

/*Matrix wiring layout:



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

/*  Joystick Library functions

Joystick.begin(bool initAutoSendState)
Joystick.end()
Joystick.sendState()

Joystick.pressButton(uint8_t button)
Joystick.releaseButton(uint8_t button)
Joystick.setButton(uint8_t button, uint8_t value)

*/