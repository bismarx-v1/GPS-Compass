#ifndef DIST_OUT_H
#define DIST_OUT_H

#include <Arduino.h>
#include "7SEG.h"

void display_distance(int distance) {
  int distance_kilo = 0; // Convert to kilometers.
  bool dp0, dp1, dp2;
  dp0 = dp1 = dp2 = false;

  if(distance > 999 && distance < 10000) { //eg: 1000m = 1.0km
    distance_kilo = distance / 10; // Convert to kilometers.
    dp2 = true;
  } else if(distance >  9999 && distance < 100000) { //eg: 10000m = 10.0km
    distance_kilo = distance / 100; // Convert to kilometers.
    dp1 = true;
  } else if(distance > 99999) {
    distance_kilo = distance / 1000; // Convert to kilometers.
    dp0 = true;
  } else {
  }
    displayDigit(2, distance_kilo % 10, dp0);
    displayDigit(1, distance_kilo % 100 / 10, dp1);
    displayDigit(0, distance_kilo / 100, dp2);
    push();

  //1000m   = 1.,0,0
  //10000m  = 1,0.,0
  //100000m = 1,0,0.
}

#endif // DIST_OUT