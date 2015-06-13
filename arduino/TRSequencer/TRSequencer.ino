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
  A Sequenced T/R Switch 
  
  This sketch senses a HIGH on pin 4 to indicate TRANSMIT
  mode.  (More on this later.) 
  
  On sensing "TRANSMIT" the controller will turn on relay 1
  connected to pin 5.  This should be used to activate
  the RF T/R relay or waveguide switch.   At the same time
  the controller will send a servo command on pin 3 (a PWM
  output) that can be used to drive a servo connected to a 
  waveguide switch.
  
  After a delay of TR2TVRT milliseconds, the controller will
  turn on relay 2 connected to pin 6.  This can be used to 
  activate the transmit side of a transverter. 
  
  After a delay of TVRT2AMP milliseconds, the controller will
  turn on relay 3 connected to pin 7.  This can be used to 
  activate a transmit amplifier. 
  
  Once the TRANSMIT mode sequence has begun, the controller
  will ignore the pin 4 input until the amplifier relay has
  been activated and AMP2REPOLL milliseconds have elapsed. 
  
  On sensing a LOW on pin 4, the controller initiates the
  sequence to get us into receive mode. 
  
  First, the amplifier relay (relay 3 on pin 7) is turned 
  off after a delay of RX2AMP milliseconds. 
  
  Then after a delay of AMP2TVRT milliseconds, relay 2 on 
  pin 6 is turned off. 
  
  Then after a delay of TVRT2TR milliseconds, relay 1 on 
  pin 5 is turned off, and the servo is returned to its
  0 setting. 
  
  
  
  Rather than connecting the arduino directly to a PTT line, 
  we use the fourth isolated relay on a 4 relay module to 
  give us a small amount of protection from big spikes on the
  PTT line.  The actual PTT line (ground to activate) is
  connected to the ground-to-activate input of relay 4. 
  The contacts of relay 4 are connected so the common
  pin connects to the arduino input pin 4.  The NO position
  is connected to ground.
*/

#include <Servo.h>

#define SERVO_PIN 3
#define PTT_SENSE 4
#define TR_SWITCH 5
#define TVRT_PTT 6
#define AMP_POWER 7

#define TR2TVRT 300
#define TVRT2AMP 100
#define AMP2REPOLL 500

#define RX2AMP 0
#define AMP2TVRT 2
#define TVRT2TR 100

Servo TR_Servo; 

void setup() {
  // put your setup code here, to run once:
  pinMode(PTT_SENSE, INPUT_PULLUP);
  pinMode(TR_SWITCH, OUTPUT);
  pinMode(TVRT_PTT, OUTPUT);
  pinMode(AMP_POWER, OUTPUT);
  
  TR_Servo.attach(SERVO_PIN);
  
  digitalWrite(TR_SWITCH, HIGH); 
  digitalWrite(TVRT_PTT, HIGH);
  digitalWrite(AMP_POWER, HIGH); 
  TR_Servo.write(0);
  TR_Servo.detach();
}

#define DEBOUNCE_DELAY 5

void loop() {
  // put your main code here, to run repeatedly:
  int ptt = digitalRead(PTT_SENSE);
  
  if(ptt == LOW) {
    delay(DEBOUNCE_DELAY);
    ptt = digitalRead(PTT_SENSE);
    if(ptt == LOW) {
      TR_Servo.attach(SERVO_PIN);
        TR_Servo.write(90);
        digitalWrite(TR_SWITCH, LOW);
        delay(TR2TVRT);
        digitalWrite(TVRT_PTT, LOW);
        delay(TVRT2AMP);
        digitalWrite(AMP_POWER, LOW); 
        delay(AMP2REPOLL); 
    }
  }
  else {
    delay(DEBOUNCE_DELAY);
    ptt = digitalRead(PTT_SENSE);
    if(ptt == HIGH) {
        TR_Servo.attach(SERVO_PIN);
        delay(RX2AMP);
        TR_Servo.write(0);
        digitalWrite(AMP_POWER, HIGH); 
        delay(AMP2TVRT);
        digitalWrite(TVRT_PTT, HIGH); 
        delay(TVRT2TR);
        digitalWrite(TR_SWITCH, HIGH);
        delay(AMP2REPOLL);
        TR_Servo.detach();
    }
  }
}
