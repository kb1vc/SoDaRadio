#include <math.h>
#include "AudioALSA.hxx"
#include <iostream>
#include <time.h>
int main(int argc, char * argv[])
{

  // SoDa::AudioALSA audio(48000, SoDa::AudioIfc::FLOAT, false);
  // SoDa::AudioALSA audioI(48000, SoDa::AudioIfc::INT16, false);
  // SoDa::AudioALSA audioI(48000, SoDa::AudioIfc::FLOAT);
  SoDa::AudioIfc * audioI = NULL; 

  if(argc < 2) exit(-1); 
  if(argv[1][0] == 'a') {
    audioI = new SoDa::AudioALSA(48000, SoDa::AudioIfc::FLOAT);
  }
  if(argv[1][0] == 'p') {
    std::cerr << "SoDaRadio no longer supports or uses Port Audio.\n"
  }
    

  float buf[1000];

#define CAPSAMPS (48 * 1000 * 5)  
  float capture_buf[CAPSAMPS]; 
  
  float ang = 0.0;
  float anginc = 2.0 * 3.14159 * 240.0 / 48000.0;
  float ai[4];
  
  int i;
  for(i = 0; i < 4; i++) {
    ai[i] = anginc * ((float) (i + 1)); 
  }
  
  int j, k;
  float again = 0.0; 

  for(k = 0; k < 16; k++) {
    again += 0.08;
    audioI->setOutGain(again);
    std::cerr << "Gain set to " << again << std::endl; 
    anginc = ai[k % 4]; 
    for(j = 0; j < 48; j++) {
      for(i = 0; i < 1000; i++) {
	buf[i] = 0.5 * sin(ang);
	// ibuf[i] = ((short) (32768.0 * buf[i])); 
	ang += anginc;
	if(ang > M_PI) ang -= (2.0 * M_PI); 
      }

      audioI->sendBufferReady(1000);
      
      audioI->send(buf, 1000);
    }
    //    audioI->sleepOut(); 

    // now read something from the mic 
    audioI->wakeIn();
    if(!audioI->recvBufferReady(1000)) {
      std::cerr << "Wow!  the recv buffer isn't ready...." << std::endl; 
    }
    audioI->recv(capture_buf, CAPSAMPS);
    audioI->sleepIn();

    //audioI->wakeOut();
    // and play the mic stuff back.
    if(!audioI->sendBufferReady(1000)) {
      std::cerr << "Wow!  the send buffer isn't ready...." << std::endl; 
    }
    audioI->send(capture_buf, CAPSAMPS);
  }



}
