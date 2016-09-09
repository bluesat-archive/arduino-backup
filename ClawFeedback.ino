#include <Wire.h>
#include "Adafruit_PWMServoDriver.h"

#define NUMBER -70
#define OTHER_NUMBER 70
            
 
int out = 0;    // variable to store the servo position 
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int servoPin = 9;
int count = 0;
int avg_pos = 0;
int avg_error = 0;
int clawCommand = 0;
int32_t frequency = 50;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup() 
{ 
  Serial.begin(9600);
  pwm.begin();
  pwm.setPWMFreq(52);
  yield();
} 
// FUCK
// 
void loop() 
{ 
  float sensorValue = 0;        // value read from the pot
  sensorValue = analogRead(analogInPin);
  sensorValue = sensorValue * 2.9412 + 400;

  /*Serial.print("Sensor Value = ");
  Serial.print(sensorValue);
  Serial.print("\n");*/
  
  int clawActual = (int)sensorValue;
  avg_pos += clawActual;
  
    // recalibrate

  
  
  if (Serial.available() > 0) {
    clawCommand = Serial.parseInt();
    Serial.print("READ INPUT\n");
  }

  if(count == 10) {
      avg_pos /= count;
      
      // send back avg_pos;
      int error_out = clawCommand - avg_pos;
      if(error_out < NUMBER || error_out > OTHER_NUMBER) {
        error_out = 0;
        out = 0;
        Serial.print("ERROR TOO BIG\n");
      } else {
        double out_inst = (double)(0.5*(double)(error_out) + avg_pos); // need to understand this
        out = (int)out_inst;
      }
      
      setPin(0, 0, out);
      //28analogWrite(servoPin, out);
      Serial.print("clawCommand = ");
      Serial.print(clawCommand);
      Serial.print("\t out = ");
      Serial.print(out);
      Serial.print("\t avg_pos = ");
      Serial.print(avg_pos);
      Serial.print("\t error_out = ");
      Serial.print(error_out);
      Serial.print("\n");
      count = 0;
      avg_pos = 0;
  }
  count++;
  // need to get desired position 
  
} 

void setPin(int port, int rand, int pwmValue) {
   pwm.setPWM(port, rand, pwmValue*(4096.0/20000.0)); 
}
