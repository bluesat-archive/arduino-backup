#include <PWM.h>


/* Sweep
 by BARRAGAN <http://barraganstudio.com> 
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Sweep
*/ 

#define NUMBER -10
#define OTHER_NUMBER 10

            
 
int out = 0;    // variable to store the servo position 
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int servoPin = 9;
int count = 0;
int avg_pos = 0;
int avg_error = 0;
int clawCommand = 0;
int32_t frequency = 50;
void setup() 
{ 

  //initialize all timers except for 0, to save time keeping functions
  InitTimersSafe(); 

  //sets the frequency for the specified pin
  bool success = SetPinFrequencySafe(servoPin, frequency);
  
  //if the pin frequency was set successfully, turn pin 13 on
  if(success) {
    pinMode(13, OUTPUT);
    digitalWrite(13, HIGH);    
  }
  
  Serial.begin(9600);
  
} 
// FUCK
// 
void loop() 
{ 
  float sensorValue = 0;        // value read from the pot
  sensorValue = analogRead(analogInPin);
  sensorValue = sensorValue * 0.03961 + 4.539;

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
        double out_inst = (double)(1*(double)(error_out) + avg_pos); // need to understand this
        out = (int)out_inst;
      }
      
      pwmWrite(servoPin, out);
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


