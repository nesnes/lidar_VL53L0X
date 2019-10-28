#ifndef BrushlessMotor_h
#define BrushlessMotor_h

#include <Arduino.h>
#define BRUSHLESS_STEPCOUNT 180

class BrushlessMotor
{
public:
  BrushlessMotor(uint8_t pinA=9, uint8_t pinB=10, uint8_t pinC=11);
  ~BrushlessMotor();
  
  void begin();
  
  void spinDegrees(float degrees);


private:
  uint8_t m_pinA;
  uint8_t m_pinB;
  uint8_t m_pinC;
  
  int m_pwmSin[BRUSHLESS_STEPCOUNT];
  
  int m_currentStepA;
  int m_currentStepB;
  int m_currentStepC;
};

#endif // BrushlessMotor_h

