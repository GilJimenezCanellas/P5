#include <iostream>
#include <math.h>
#include "seno.h"
#include "keyvalue.h"
#include <errno.h>
#include <string.h>

#include <stdlib.h>

using namespace upc;
using namespace std;

Seno::Seno(const std::string &param) 
  : adsr(SamplingRate, param) {
  bActive = false;
  x.resize(BSIZE);

  /*
    You can use the class keyvalue to parse "param" and configure your instrument.
    Take a Look at keyvalue.h    
  */
  KeyValue kv(param);
  
  /////////////////////////
  //     TAULA FITXER    //
  /////////////////////////
  std::string file_name = "period2.wav";
  static string kv_null;
  /*
  if ((file_name = kv("guitar.wav")) == kv_null) {
    cerr << "Error: no se ha encontrado el campo con el fichero de la señal para un instrumento FicTabla" << endl;
    throw -1;
  }
  */
  unsigned int fm;

  if (readwav_mono(file_name, fm, tbl) < 0) {
    
    cerr << "Error: no se puede leer el fichero " << file_name << " para un instrumento FicTabla" << " (" << strerror(errno) << ")"<< endl;
    throw -1;
  }


  N = tbl.size();
  phase = 0;
  index = 0;
  step1 = 2 * M_PI /(float) N;

  
}


void Seno::command(long cmd, long note, long vel) {
  if (cmd == 9) {		//'Key' pressed: attack begins
    bActive = true;
    adsr.start();
    float f0 = 440.0 * pow(2 ,((float)note-69.0)/12.0);
    nota = f0/SamplingRate;
	  A = vel / 127.;
    step2 = 2 * M_PI * nota;
    phase = 0;
    index = 0;
  }
  else if (cmd == 8) {	//'Key' released: sustain ends, release begins
    adsr.stop();
  }
  else if (cmd == 0) {	//Sound extinguished without waiting for release to end
    adsr.end();
  }
}


const vector<float> & Seno::synthesize() {
  if (not adsr.active()) {
    x.assign(x.size(), 0);
    bActive = false;
    return x;
  }
  else if (not bActive)
    return x;

  for (unsigned int i=0; i<x.size(); ++i) {
    
    if(phase == (int)phase){
      
      x[i] = 0.15*A*tbl[index]; 
   
    }else{

      int index1 = (int)ceil(phase);
      int index2 = (int)floor(phase);
      float alpha1 = (float) index2 - phase;
      float alpha2 = (float) phase - index1;

      while(index2 >= N){

        index2 -= N;
      }
      while(index1 >= N){

        index1 -= N;
      }

      x[i] = 0.15*A*(tbl[index1]*(alpha1) + tbl[index2]*(alpha2));

    }
    phase += step2/step1;
    index = phase;

    while(index >= N){
      phase = step2/step1;
      index -= N;
    }
  } 
  adsr(x); //apply envelope to x and update internal status of ADSR

  return x;
}