#ifndef DISTANCEMEASURE_H
#define DISTANCEMEASURE_H

#include <Arduino.h> 

#define TRIG_PIN D1
#define ECHO_PIN D2

#define NUM_SAMPLES 30   // number of readings per measurement
#define DISCARD_RATIO 0.2 // discard top & bottom 20% of values

void initializeSensor();

int getDistanceCM();
int getDistanceMM();
int getPerfectDistanceMM();

#endif