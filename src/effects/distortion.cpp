
#include <iostream>
#include <math.h>
#include "distortion.h"
#include "keyvalue.h"

#include <stdlib.h>

using namespace upc;
using namespace std;

static float SamplingRate = 44100;

Distortion::Distortion(const std::string &param) {
  KeyValue kv(param);

  if (!kv.to_float("T", T))
    T = 0.7; //default value
}

void Distortion::command(unsigned int comm) {}

void Distortion::operator()(std::vector<float> &x){
  for (unsigned int i = 0; i < x.size(); i++) {
      if (x[i] > T){
          x[i] = T;
      }
      else if (x[i] < -T){
          x[i] = -T;
      }
  }
}
