/*
Copyright (c) 2015, Matthew H. Reilly (kb1vc)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in
    the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 Switch Monitor
 
 This arduino sketch is used to control a bank of relays activated
 by outputs on pins 4 through 11. 
 
 The original purpose was to use this as a general purpose controller
 for a Transmit/Receive and bank switch in association with the SoDaRadio
 software defined transciever develope for the Ettus Research USRP 
 family of radios.  
 
 The monitor listens on the USB/serial port for commands of the form 
 
 UPddPU<CR>
 
 where <d> is a digit from 0 to 7
 UP energizes the relay coil. 
 
 DNddDN<CR>
 
 DN deenergizes the relay coil. 
 
 The commands are UP and DN instead of ON and OFF because it
 seems like a good idea at this time. 
 
 Every command must be a palindrome, or it will be rejected. 
 This is to prevent crazy action on the relays when a random stream
 of serial junk is sent to the controller. 
 
 The controller listens at 115200 baud.  This requires the use
 of the hardware serial port.  The Ettus N200 provides a pipe
 from UDP socket 49172 to the RS232 GPS port.  
 
 It responds to each command with a non-palindromic
 
 OK [command string]<CR>
 
 where the input command string is repeated. 
 
 or
 
 BAD [command string]<CR> 
 
 where the input command string is repeated, but the command is ignored.
 
 */
 
#define FIRST_PIN 4
 // baud rate for N200 exp port -- exp doesn't appear to work...
 //#define BAUD_RATE 230400
 // baud rate for N200 GPS port
#define BAUD_RATE 115200
 
 void setup()
 {
   int k;
   // start the serial port baud rate
   Serial.begin(BAUD_RATE);
   while (!Serial) {
   }
   
   // put the control pins in input mode
   for(k = FIRST_PIN; k <= FIRST_PIN + 8; k++) {
     pinMode(k, OUTPUT);
    
    digitalWrite(k, HIGH); 
   }
   
   pinMode(13, OUTPUT);
 }
 
 int inByte; 
 int state;
 char cmdbuf[6];
 byte bidx;
 
 void badBeef()
 {
   bidx = 0;
   Serial.print("BAD [");
   Serial.print(cmdbuf);
   Serial.println(']');
   int i;
 }
 
 void goodBeef()
 {
   bidx = 0;
   Serial.print("OK [");
   Serial.print(cmdbuf);
   Serial.println(']');
 }
 
 void sendBlink(int c)
 {
   return;
   int i;
   for(i = 0; i < c; i++) {
     digitalWrite(13, HIGH); 
     delay(500);
     digitalWrite(13, LOW);
     delay(500);
   }
 }
 
 void loop()
 {
   if (Serial.available() > 0) {
     inByte = Serial.read();
     
     if ((inByte == 0xa) || (inByte == 0xd)) {// CR
       if(bidx != 6) {
         sendBlink(2);
         delay(2);
         sendBlink(bidx);
         badBeef();
         bidx = 0;
         return;
       }
       for(bidx = 0; bidx < 3; bidx++) {
         if(cmdbuf[bidx] != cmdbuf[5 - bidx]) {
           sendBlink(3);
           badBeef();
           return;
         }
       }
       
       if((cmdbuf[0] == 'S') && (cmdbuf[1] == 'T')) {
         state = HIGH;
         sendBlink(4);
         goodBeef();
       }
       else if((cmdbuf[0] == 'O') && (cmdbuf[1] == 'N')) {
         state = LOW;
         sendBlink(4);
         goodBeef();
       }
       else {
         sendBlink(5);
         badBeef();
         return;
       }
       
       sendBlink(8);
       // if we get here, then we need to find out which pin was tickled
       int b0 = cmdbuf[2];
       digitalWrite(FIRST_PIN + (b0 - 0x30), state);       
       bidx = 0;
     }
     else {
       cmdbuf[bidx] = inByte & 0xff; 
       if(bidx == 6) {
         sendBlink(6);
         bidx = 0;
       }
       else {
         bidx = bidx + 1;
       }
     }
   }
 }
