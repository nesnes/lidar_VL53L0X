#include "BrushlessMotor.h"

BrushlessMotor::BrushlessMotor(uint8_t pinA, uint8_t pinB, uint8_t pinC)
	: m_pinA(pinA)
	, m_pinB(pinB)
	, m_pinC(pinC)
{
	
}

BrushlessMotor::~BrushlessMotor(){
	
}
  
void BrushlessMotor::begin(){
  /* https://playground.arduino.cc/Main/TimerPWMCheatsheet/
   * For Arduino Mega: (tested on Arduino Mega 2560)
  timer 0 (controls pin 13, 4) If you change TCCR0B, it affects millis() and delay().
  timer 1 (controls pin 12, 11)
  timer 2 (controls pin 10, 9)
  timer 3 (controls pin 5, 3, 2)
  timer 4 (controls pin 8, 7, 6)
  TCCRnB, where 'n' is the number for the timer*/
  
  /*For Arduino pRO mINI:
  timer 0 (controls pin 5, 6) If you change TCCR0B, it affects millis() and delay().
  timer 1 (controls pin 9, 10)
  timer 2 (controls pin 11, 3)
  TCCRnB, where 'n' is the number for the timer*/
  
	TCCR1B = TCCR1B & 0b11111000 | 0x01; // set PWM frequency @ 31250 Hz for Pins 9 and 10
	TCCR2B = TCCR2B & 0b11111000 | 0x01; // set PWM frequency @ 31250 Hz for Pins 11 and 3 (3 not used)
	ICR1 = 255 ; // 8 bit resolution, required?
	//  ICR1 = 1023 ; // 10 bit resolution


	m_currentStepA = (int)(((float)(BRUSHLESS_STEPCOUNT)/3.0f)*0.0f);
	m_currentStepB = (int)(((float)(BRUSHLESS_STEPCOUNT)/3.0f)*1.0f);
	m_currentStepC = (int)(((float)(BRUSHLESS_STEPCOUNT)/3.0f)*2.0f);

	for(int i=0;i<BRUSHLESS_STEPCOUNT;i++){
		m_pwmSin[i] = 127.0f + 127.0f*sin( ((2.0f*PI)/(float)(BRUSHLESS_STEPCOUNT)) *(float)(i) );
	}

	pinMode(m_pinA, OUTPUT);
	pinMode(m_pinB, OUTPUT);
	pinMode(m_pinC, OUTPUT);
}

void BrushlessMotor::spinDegrees(float degrees)
{
  int sleep = 500;//400
  float currDegrees=0;
  unsigned long now = micros();
  
	while((degrees>0 && currDegrees < degrees) || (degrees<=0 && currDegrees > degrees)){
  
		//if((now - lastMotorDelayTime) >  motorDelayActual){ // delay time passed, move one step 
		int increment;
		if (degrees>0) increment = 1;
		else increment = -1;  
		/*
		if(sleep<-300) increment*=5;
		else if(sleep<-200) increment*=4;
		else if(sleep<-100) increment*=3;
		else if(sleep<0) increment*=2;
		*/
		m_currentStepA = m_currentStepA + increment;
		if(m_currentStepA > BRUSHLESS_STEPCOUNT-1) m_currentStepA = 0;
		if(m_currentStepA<0) m_currentStepA = BRUSHLESS_STEPCOUNT-1;
		 
		m_currentStepB = m_currentStepB + increment;
		if(m_currentStepB > BRUSHLESS_STEPCOUNT-1) m_currentStepB = 0;
		if(m_currentStepB<0) m_currentStepB = BRUSHLESS_STEPCOUNT-1;
		 
		m_currentStepC = m_currentStepC + increment;
		if(m_currentStepC > BRUSHLESS_STEPCOUNT-1) m_currentStepC = 0;
		if(m_currentStepC<0) m_currentStepC = BRUSHLESS_STEPCOUNT-1;
	   
		//lastMotorDelayTime = now;
		//}  
		analogWrite(m_pinA, m_pwmSin[m_currentStepA]/1.5);
		analogWrite(m_pinB, m_pwmSin[m_currentStepB]/1.5);
		analogWrite(m_pinC, m_pwmSin[m_currentStepC]/1.5);
		if(sleep>0)
			delayMicroseconds(sleep);
		currDegrees += (degrees>0)?0.275:-0.275;
	}
}

