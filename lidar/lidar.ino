/*
 * This source code controls a brushless motor and 6 VL530X.
 * The rotates motor up to pushing on an end-course button To calibrate its orientation.
 * Then it start ocillating while performing distance measures with the sensors.
 * The result are sent on the serial monitor as a list of angles and distances.
*/

#//define MODE_I2C
#ifndef MODE_I2C
  #define MODE_SERIAL
  //#define SERIAL_DEBUG
#endif
#include "comunication.h"

#include <Wire.h>
#include "SoftwareWire.h"
#include "VL53L0X.h"
#include "BrushlessMotor.h"
SoftwareWire* wireObj;

#define SENSOR_COUNT 6
int sensorAngle[SENSOR_COUNT] = {0, 60, 120, 180, 240, 300};
int sensorAddress[SENSOR_COUNT] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35};
int sensorDisabled[SENSOR_COUNT] = {0, 0, 0, 0, 0, 0};
int sensorEnablePin[SENSOR_COUNT] = {12, 13, A0, A1, A2, A3};
VL53L0X sensors[SENSOR_COUNT];
BrushlessMotor motor;

#define CALIBRATION_SWITCH_PIN 8


// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

//#define LONG_RANGE

#define CONTINUOUS_MODE


// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR (distance measure)
// - higher accuracy at the cost of lower speed (distance measure)

#define HIGH_SPEED
//#define HIGH_ACCURACY


void initialCalibration(){
  //Move away from end stop
  motor.spinDegrees(20);
  delay(250);
  //Find end stop approximately
  while(digitalRead(CALIBRATION_SWITCH_PIN)==LOW){
    motor.spinDegrees(-1, 1);
    delay(50);
  }
  //Move away from end stop
  motor.spinDegrees(5);
  delay(250);
  //Find end stop precisely
  while(digitalRead(CALIBRATION_SWITCH_PIN)==LOW){
    motor.spinDegrees(-0.2, 1);
    delay(50);
  }
  //Move to start position
  motor.spinDegrees(2, 1);
}

void setup()
{
  pinMode(CALIBRATION_SWITCH_PIN, INPUT);
  comunication_begin(8);//I2C address 8

  //Disable Sensors
  for(int i=0;i<SENSOR_COUNT;i++){
    pinMode(sensorEnablePin[i], OUTPUT);
    digitalWrite(sensorEnablePin[i], LOW);
  }
  delay(10);
  for(int i=0;i<SENSOR_COUNT;i++){
    if(sensorDisabled[i]) continue;
    digitalWrite(sensorEnablePin[i], HIGH);
  }
  delay(10);  
  for(int i=1;i<SENSOR_COUNT;i++){
    if(sensorDisabled[i]) continue;
    digitalWrite(sensorEnablePin[i], LOW);
  }
  delay(10); 
  
  wireObj = new SoftwareWire(4,5);
  wireObj->begin();

  //Set sensor address
  for(int i=0;i<SENSOR_COUNT;i++){
    if(sensorDisabled[i]) continue;
    digitalWrite(sensorEnablePin[i], HIGH);
    delay(15);
    sensors[i].setWire(wireObj);
    sensors[i].setAddress(sensorAddress[i]);
    sensors[i].init();
    delay(11);
  }

  //Configure sensors
  for(int i=0;i<SENSOR_COUNT;i++){
    if(sensorDisabled[i]) continue;
    sensors[i].setTimeout(100);
    #if defined LONG_RANGE
      // lower the return signal rate limit (default is 0.25 MCPS)
      sensors[i].setSignalRateLimit(0.1);
      // increase laser pulse periods (defaults are 14 and 10 PCLKs)
      sensors[i].setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
      sensors[i].setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
    #endif
    
    #if defined HIGH_SPEED
      // reduce timing budget to 20 ms (default is about 33 ms)
      sensors[i].setMeasurementTimingBudget(20000);
    #elif defined HIGH_ACCURACY
      // increase timing budget to 200 ms
      sensors[i].setMeasurementTimingBudget(200000);
    #endif

    #if defined CONTINUOUS_MODE
      sensors[i].startContinuous();
    #endif
    
  }

  //Init motor
  motor.begin();  
  initialCalibration();
}

#define MEASURE_COUNT 3*SENSOR_COUNT 
#define MEASURE_STEP 20
uint8_t currentMeasures[360];//cm

void executeOrder(){
  comunication_read();
  if(comunication_msgAvailable()){
    if(comunication_InBuffer[0] == '\0'){
      //ignore
    }
    else if(comunication_InBuffer[0] == '#' && comunication_InBuffer[1] != '\0'){
      //ignore
    }
    else if(strstr(comunication_InBuffer, "id")){
      sprintf(comunication_OutBuffer, "CollisionDetectionLidar");//max 29 Bytes
      comunication_write();//async
    }
    else if(strstr(comunication_InBuffer, "distances get")){
      //D Step A1 A2... in binary, not text, no separator
      comunication_OutBuffer[0] = 'D';
      comunication_OutBuffer[1] = MEASURE_STEP;
      for(int j=0;j<MEASURE_COUNT;j++){
        int angle = j*MEASURE_STEP;
        if(angle>=360) angle = 0;
        uint8_t val = currentMeasures[angle];
        if(val==0) val = 1;//avoid unexpected \0 in string
        if(val==COMUNICATION_START_BYTE) val = COMUNICATION_START_BYTE-1;//avoid unexpected START BYTE in string
        if(val==COMUNICATION_END_BYTE) val = COMUNICATION_END_BYTE-1;//avoid unexpected END BYTE in string
        comunication_OutBuffer[j+2] = val;
        #ifdef SERIAL_DEBUG
          Serial.print(angle);
          Serial.print(" ");
          Serial.print(val);
          Serial.print("\n");
        #endif
      }
      comunication_write();//async
    }
    else{
      sprintf(comunication_OutBuffer,"ERROR");
      comunication_write();//async
    }
    comunication_cleanInputs();
  }
}

int curr_angle=0;
int dir=1;
void loop()
{
  //Read commands
  executeOrder();

  //Spin and measure
  if(curr_angle%MEASURE_STEP==0){
      for(int j=0;j<SENSOR_COUNT;j++){
        uint16_t value=0;
        int angle=sensorAngle[j] + curr_angle;
        if(angle>=360) angle = 0;
        
        if(sensorDisabled[j]) continue;
        
        #if defined CONTINUOUS_MODE
          value=sensors[j].readRangeContinuousMillimeters();
        #else
          value=sensors[j].readRangeSingleMillimeters();
        #endif
        value/=10;//to cm
        if (sensors[j].timeoutOccurred()) { value=0; }
        else value = currentMeasures[angle]*0.5 + value*0.5;
        currentMeasures[angle] = value;
        /*if(j>0) Serial.print(";");
        Serial.print(angle);
        Serial.print(" ");
        Serial.print(value);*/
      }
      //Serial.println();
  }
  motor.spinDegrees(dir);
  curr_angle+=dir;
  if(curr_angle>60) dir=-1;
  if(curr_angle<0) dir=1;
  delay(2);
}
