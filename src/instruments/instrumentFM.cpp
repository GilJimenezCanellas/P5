
#include <iostream>
#include <math.h>
#include "instrumentFM.h"
#include "keyvalue.h"
#include <errno.h>
#include <string.h>

#include <stdlib.h>

using namespace upc;
using namespace std;

InstrumentFM::InstrumentFM(const std::string &param) 
  : adsr(SamplingRate, param) {
  bActive = false;
  x.resize(BSIZE);

  /*
    You can use the class keyvalue to parse "param" and configure your instrument.
    Take a Look at keyvalue.h    
  */
  KeyValue kv(param);
  
  
  /////////////////////////
  //     TAULA NORMAL    //
  /////////////////////////

  if (!kv.to_int("N",N))
    N = 40; //default value

  if (!kv.to_float("N1",N1))
    N1 = 5; //default value
  if (!kv.to_float("N2",N2))
    N2 = 6; //default value
  if (!kv.to_float("I",I))
    I = 1; //default value
  I = 1. - pow(2, -I / 12.);
  
  //Create a tbl with one period of a sinusoidal wave
  tbl.resize(N);
  float fase = 0;
  phase = 0;
  index = 0;
  float step1 = 2 * M_PI /(float) N;
  //index = 0;
  for (int i=0; i < N ; ++i) {
    tbl[i] = sin(fase);
    fase += step1;
  }
}


void InstrumentFM::command(long cmd, long note, long vel) {
  if (cmd == 9) {		//'Key' pressed: attack begins
    bActive = true;
    adsr.start();
    float fc = 440.0 * pow(2 ,((float)note-69.0)/12.0);
    //cout<<fc<<endl;
    nota = fc/SamplingRate;
    fm = (N2/N1) * nota;
	A = vel / 127.;
    substep1 = 2 * M_PI * nota;
    substep2 = 2 * M_PI * fm;
    //cout<<nota<<", "<<step2/step1<<endl;
    phase = phase1 = phase2 = 0;
    index = 0;
  }
  else if (cmd == 8) {	//'Key' released: sustain ends, release begins
    adsr.stop();
  }
  else if (cmd == 0) {	//Sound extinguished without waiting for release to end
    adsr.end();
  }
}


const vector<float> & InstrumentFM::synthesize() {
  if (not adsr.active()) {
    x.assign(x.size(), 0);
    bActive = false;
    return x;
  }
  else if (not bActive)
    return x;

  for (unsigned int i=0; i<x.size(); ++i) {
    

   x[i] = 0.85*A*sin(phase1 + I*sin(phase2));
   phase1 += substep1;
    
    while(phase1 >= 2*M_PI){
        phase1 -= 2*M_PI;
    }

    phase2 += substep2;

    while(phase2 >= 2*M_PI){
        phase2 -= 2*M_PI;
    }
  } 
  adsr(x); //apply envelope to x and update internal status of ADSR

  return x;
}
