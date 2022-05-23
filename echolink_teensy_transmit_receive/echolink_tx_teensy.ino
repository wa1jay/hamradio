
// This simple program allows you to control your radio's transmit from echolink using a Teensy microcontroller over it's USB COM port on windows.  
// Set your PTT Activation to 'ASCII Serial' in the Tools->Sysop Setting... menu on the 'TX Ctl' Tab
// Check the 9600 bps box, and select your serial port in the 'Serial Port' Dropdown.
//
// 
// Echolink sends an ascii 'R' when it changes to Receive mode and 'T' when it starts transmitting.
//
// Make sure you select a 'serial' configuration in the Tools->USB Type Menu in the Arduino IDE before compiling.
//
// To swap the transmit and receive logic, swap the HIGH and LOW below in the digitalWrite function calls and recompile.
//
// To change the output pin, consult https://www.pjrc.com/teensy/pinout.html for your teensy to select an output pin, and
//  change the led value below from 13 to your new pin number.
//
// You should protect your radio by using an optical coupler like a 4N25 and not wire the Teensy output directly to your radio PTT.
//
// I use the led pin because it provides visual feedback that echolink is transmitting.  Use whatever pin works for you.
//
// You can change the COM port number of the Teensy by opening windows Device Manager, expanding 'Ports (COM & LPT)', right clicking
//  on the com port the teensy is using, click properties, go to the 'port settings' tab and clieck the 'Advanced' button.  Pick a 
//  new COM port in the range between COM1 to COM8.
//
// You can email me with questions using my email link on QRZ, https://www.qrz.com/db/WA1JAY
// 73 
// de WA1JAY
//

char incomingByte;
int led = 13;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(led, OUTPUT);  
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    incomingByte = Serial.read();  // will not be -1
    // swap HIGH and LOW below to reverse logic.
    if (incomingByte == 'T')
    {
      digitalWrite(led, HIGH); 
    }
    if (incomingByte == 'R')
    {
      digitalWrite(led, LOW); 
    }

  }

}
