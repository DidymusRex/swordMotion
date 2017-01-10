//
// swordMotion.ino
//

// accelerometer stuff
#include <Wire.h> // Must include Wire library for I2C

// There are a few options when it comes to initializing the MMA8452Q:
// param 1: scale (2G is default)
//   SCALE_2G, SCALE_4G, or SCALE_8G
// param 2: output data rate in Hz (800 is default)
//   ODR_800, ODR_400, ODR_200, ODR_100, ODR_50, ODR_12, ODR_6, or ODR_1. 
//   Sets to 800, 400, 200, 100, 50, 12.5, 6.25, or 1.56 Hz.
// Includes the SFE_MMA8452Q library
#include <SparkFun_MMA8452Q.h>
MMA8452Q accel;

// millis at last accel.read()
long int acc_time;
// usec between accel.read()
const int acc_usec = 10;

// arrays for smoothing. avarage cx,cy, and cz over asize reads
const int asize = 10;
float xx[asize]={};
float yy[asize]={};
float zz[asize]={};
// "smooth" reading
float sx, sy, sz;

// calculate slope every 100 usec (10x per second)
const int slope_usec=100;
long int lm;
float slope_x, slope_y, slope_z, lx, ly, lz; 

// button stuff
#define BTNPIN 10
// millis elapsed to debounce
const int btn_usec = 10;
// button stat, current value of input pin, and counter for debounce
int btn_stat, btn_read, btn_cntr;
// millis at last button status change
long int btn_time;

// -----------------------------------------------------------------------------
void setup()
{
  //
  // set up serial commuication
  // ----------------------------------- 
  Serial.begin(9600);
  Serial.println("swordMotion Test Code!");

  //
  // init with default scale and speed
  // ----------------------------------- 
  accel.init(SCALE_2G, ODR_800);

  //
  // set up the button
  // ----------------------------------- 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BTNPIN, INPUT);
  btn_cntr=0;
}

// -----------------------------------------------------------------------------
void loop()
{
  // debounce button over 10 usec
  // ----------------------------------- 
  // we have entered a new usec
  if (millis() > btn_time){
    // read pin
    btn_read = digitalRead(BTNPIN);
    
    // pin matches stored state and we previosly detected a state change
    // this is a bounce, decrement the counter.
    if (btn_read == btn_stat && btn_cntr > 0) btn_cntr--;
    
    // pin (still) differs from stored state. increment the counter
    if (btn_read != btn_stat) btn_cntr++;
    
    // pin has differed from stored state for 10 usec
    // zero the counter and change stored state.
    if (btn_cntr >= btn_usec) {
      btn_cntr=0;
      btn_stat=btn_read;
      digitalWrite(LED_BUILTIN, btn_stat);
    }
    
    // get current usec
    btn_time=millis();
  }

  // only output when button is pressed. Button is low by default (pull-down resistor)
  if (btn_stat == HIGH) {
    //
    readAccel();
    
    // compute slopes
    computeSlopes();
  }
}

// -----------------------------------------------------------------------------
//
void readAccel(){
  // read accel every acc_usec millis
  if (millis() > acc_time + acc_usec) {
    if (accel.available()) {
      accel.read();
      smoothReading();
      
      printSmoothed();
      Serial.println();
 
      acc_time=millis();
    }
  }
}

void computeSlopes(){
  if (millis() > lm + slope_usec){
    long int m = millis();

    // compute slopes
    slope_x=(sx-lx)*1000/(m-lm);
    slope_y=(sy-ly)*1000/(m-lm);
    slope_z=(sz-lz)*1000/(m-lm);

    // save current as last
    lm=m; lx=sx; ly=sy; lz=sz;
    //printSlope();
    //Serial.println();
  }  
}

void smoothReading(){
  float tx=accel.cx, ty=accel.cy, tz=accel.cz;

  // total and shift arrays
  for (int i=0; i<(asize-1); i++){
    // total
    tx += xx[i]; ty += yy[i]; tz += zz[i];
    // shift
    xx[i]=xx[i+1]; yy[i]=yy[i+1]; zz[i]=zz[i+1];
  }
  // push new values on end of arrays
  xx[asize]=accel.cx; yy[asize]=accel.cy; zz[asize]=accel.cz;
  // smooth is average of array
  sx=tx/asize; sy=ty/asize; sz=tz/asize;
}

void printSmoothed(){
  Serial.print(sx*10, 3);
  Serial.print("\t");
  Serial.print(sy*10, 3);
  Serial.print("\t");
  Serial.print(sz*10, 3);
}

void printSlope(){
  Serial.print(slope_x, 3);
  Serial.print("\t");
  Serial.print(slope_y, 3);
  Serial.print("\t");
  Serial.print(slope_z, 3);
}

// -----------------------------------------------------------------------------
//

