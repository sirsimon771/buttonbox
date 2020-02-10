#include <Arduino.h>
#include <Joystick.h>

int i = 0;

void setup() {

  Serial.begin(9600);

}

void loop() {

  delay(750);
  Serial.print("\r\nHello World! - ");
  Serial.print(i);
  i++;
}
