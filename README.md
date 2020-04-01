# buttonbox
An Arduino library to use two ProMicros to control a simracing buttonbox

the buttonbox uses 12 pushbuttons, 5 on-off toggle switches, 5 momentary-3-position switches and 5 rotary encoders with pushbuttons.

everything except the rotary encoders turning functionality is organized into a 6x6 matrix like in a keyboard, to only use 12 I/O pins for the 32 inputs.
This matrix is being read by the "main" arduino ProMicro, which is connected to the PC via USB and acts as a gaming controller in Windows, through the Joystick Library which can be found here: https://github.com/MHeironimus/ArduinoJoystickLibrary.git
The 5 rotary encoders are being continuously read by the "sub" arduino, as it appears to be more reliable than using interrupts. The secondary arduino then sends one byte of information containing the number of the pressed button (0-9, two buttons per encoder one each direction) and the desired state of this button (press/release).
The main board then acts on the received information and presses the specified button on the virtual joystick.

The "sub" board is the I2C master, sending new information whenever a rotation is detected.
The "main" board is the I2C slave, listening for traffic from the "sub" arduino while also simultaneously reading the button matrix.
