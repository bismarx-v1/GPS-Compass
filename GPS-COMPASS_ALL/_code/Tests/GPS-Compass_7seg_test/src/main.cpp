#include "LedControl.h"
#include <Arduino.h>

//pin 12 is connected to the DataIn 
//pin 11 is connected to the CLK 
//pin 10 is connected to LOAD 
//We have only a single MAX72XX.

LedControl lc=LedControl(12,11,10,1);

unsigned long delaytime=1;

void setup() {
  /*
   The MAX72XX is in power-saving mode on startup,
   we have to do a wakeup call
   */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);
}

int time2 = 0;
int s0 = 0;
int s1 = -1;
int s2 = -1;
int s3 = -1;
int s4 = -1;
int s5 = -1;
int s6 = -1;
int s7 = -1;
void loop() { 
  
  s0=time2;
  
  if (time2==9){
    time2=-1;
  }
  
  if (s0==0){
    s1=s1+1;
  }
  
  //---------------------------------------------------------
  
    if (s1==10){
    s1=0;
  }
  
  if (s1==0&&s0==0){
    s2=s2+1;
  }
  
  //---------------------------------------------------------
  
     if (s2==10){
    s2=0;
  }
  
  if (s2==0&&s1==0&&s0==0){
    s3=s3+1;
  }
  
    //---------------------------------------------------------
  
     if (s3==10){
    s3=0;
  }
  
  if (s3==0&&s2==0&&s1==0&&s0==0){
    s4=s4+1;
  }
  
      //---------------------------------------------------------
  
     if (s4==10){
    s4=0;
  }
  
  if (s4==0&&s3==0&&s2==0&&s1==0&&s0==0){
    s5=s5+1;
  }
  
        //---------------------------------------------------------
  
     if (s5==10){
    s5=0;
  }
  
  if (s5==0&&s4==0&&s3==0&&s2==0&&s1==0&&s0==0){
    s6=s6+1;
  }
  
          //---------------------------------------------------------
  
     if (s6==10){
    s6=0;
  }
  
  if (s6==0&&s5==0&&s4==0&&s3==0&&s2==0&&s1==0&&s0==0){
    s7=s7+1;
  }
  
  
  lc.setChar(0,0,s0 ,false);
  lc.setChar(0,1,s1 ,false);
  lc.setChar(0,2,s2 ,false);
  lc.setChar(0,3,s3 ,false);
  lc.setChar(0,4,s4 ,false);
  lc.setChar(0,5,s5 ,false);
  lc.setChar(0,6,s6 ,false);
  lc.setChar(0,7,s7 ,false);
  
   delay(delaytime);
  time2=time2+1;
}