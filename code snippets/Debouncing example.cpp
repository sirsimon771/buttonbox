/* Debounce a push button

  This sketch will demonstrate debouncing a pushbutton with software.
  Every time the button is pressed the LED will toggle

  The circuit:
   LED attached from pin 13 to ground
   pushbutton attached from pin 2 to +5V
   10K resistor attached from pin 2 to ground

   Note: On most Arduino boards, there is already an LED on the board
  connected to pin 13, so you don't need any extra components for this example.
  created 21 November 2006
  by David A. Mellis
  modified 30 Aug 2011
  by Limor Fried
  modifies again sometime in Sept 2012
  by Michael J
  This example code is in the public domain.
  http://www.arduino.cc/en/Tutorial/Debounce
  http://www.opensourcehardwarejunkies.com
*/

#include<Arduino.h>

//initialize and declare variables
const int ledPin = 13; //led attached to this pin
const int buttonPin = 2; //push button attached to this pin

int buttonState = LOW; //this variable tracks the state of the button, low if not pressed, high if pressed
int ledState = -1; //this variable tracks the state of the LED, negative if off, positive if on

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers


void setup() {

  //set the mode of the pins...
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);

}//close void setup


void loop() {

  //sample the state of the button - is it pressed or not?
  buttonState = digitalRead(buttonPin);

  //filter out any noise by setting a time buffer
  if ( (millis() - lastDebounceTime) > debounceDelay) {

    //if the button has been pressed, lets toggle the LED from "off to on" or "on to off"
    if ( (buttonState == HIGH) && (ledState < 0) ) {

      digitalWrite(ledPin, HIGH); //turn LED on
      ledState = -ledState; //now the LED is on, we need to change the state
      lastDebounceTime = millis(); //set the current time
    }
    else if ( (buttonState == HIGH) && (ledState > 0) ) {

      digitalWrite(ledPin, LOW); //turn LED off
      ledState = -ledState; //now the LED is off, we need to change the state
      lastDebounceTime = millis(); //set the current time
    }//close if/else

  }//close if(time buffer)

}//close void loop