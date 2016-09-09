
#include <Wire.h>

#include <Arduino.h>
#include "Adafruit_PWMServoDriver.h"
#include "message.h"
#include "pins.h"

#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

bool recieveMsg();
void sendMsg(toNucAdapter msg);
// Initialise the pwm board, note we may need to change the address
// see examples
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

char buffer[sizeof(toControlMsg)];
char bytesRead;


/********** COPY FROM CLAW FEEDBACK ******************/

#define NUMBER -70
#define OTHER_NUMBER 70
#define CLAWPIN 0 // depends on which pinit's attached to on i2c to pwm module


int out = 0;    // variable to store the servo position
const int analogInPin = A0;  // Claw feedback pin.
int count = 0;
int avg_pos = 0;
int avg_error = 0;
int clawCommand = 0;
int32_t frequency = 50;


/*****************************************************/


void setup() {
    //Serial.begin(19200); //this is the speed specified on line 109 of Bluetongue.cpp I think it is correct

    mySerial.begin(19200);

    bytesRead = 0;
    pwm.begin();

    pwm.setPWMFreq(52);

/********** COPY FROM CLAW FEEDBACK ******************/
   yield(); // not 100% on this
/*****************************************************/

}
bool foundFirst = false;
toMsgAdapter msg;
void loop() {
    
    toNucAdapter fromMsg;
    
    
    byte MAGIC[2] = {0x55, 0xAA};
    
    while(mySerial.available() > 0) {
        char val = mySerial.read();
        if(val == MAGIC[0]) {
          bytesRead = 0;
          foundFirst = true;
        } /*else if(foundFirst && val == MAGIC[1]) {
          bytesRead = 1;
          foundFirst = false;
        } else {
          foundFirst = false;
        }*/
        
        if(foundFirst) {
          msg.data.structBytes[++bytesRead] = val;
          if(bytesRead >=  sizeof(struct toControlMsg))  {
              break;
          }
        }
    }

    //TODO: add adc code here

    /********** COPY FROM CLAW FEEDBACK ******************/
    float sensorValue = 0;        // value read from the pot
    sensorValue = analogRead(analogInPin);
    sensorValue = sensorValue * 2.9412 + 400; // calibration numbers


    int clawActual = (int)sensorValue;
    avg_pos += clawActual;

    if(count == 10) {
       avg_pos /= count;
       // I think it's fromMsg or whatever
       fromMsg.msg.clawActual = avg_pos; // not sure if this correct
       int error_out = clawCommand - avg_pos;
       if(error_out < NUMBER || error_out > OTHER_NUMBER) {
          error_out = 0;
          out = 0;
          
       } else {
          double out_inst = (double)(0.5*(double)(error_out) + avg_pos); // need to understand this
          out = (int)out_inst;
       }

       setPin(CLAWPIN, 0, out); // 
       avg_pos = 0;
       count = 0;
    }
    count++;

/*****************************************************/
    //fromMsg.msg.pot0 = bytesRead;
    //sendMsg(fromMsg);
    msg.success = recieveMsg();
    
    if (bytesRead > 0) {    
      
        //note: we may want to wrap this with our saftey caps like it is on the board
        //I'm not 100% that this set PWM function actually sets a us pulse width, need to double check

        setPin(FL_SPEED, 0, msg.data.msg.flSpeed);
        setPin(BL_SPEED, 0, msg.data.msg.blSpeed);
        setPin(FR_SPEED, 0, msg.data.msg.frSpeed);
        setPin(BR_SPEED, 0, msg.data.msg.brSpeed);


        setPin(FL_ANG, 0, msg.data.msg.flAng);
        setPin(FR_ANG, 0, msg.data.msg.frAng);


        setPin(ARM_ROT, 0, msg.data.msg.armRotate);
        //setPin(ARM_ROT, 0, 1500);
        setPin(ARM_TOP, 0, msg.data.msg.armTop);
        setPin(ARM_BOT, 0, msg.data.msg.armBottom);

        //These two + grip are 7.4V
        setPin(CLAW_ROT, 0, msg.data.msg.clawRotate);
        setPin(LIDAR_TILT, 0, msg.data.msg.lidarTilt);

        // do claw grip stuff here

        if(clawCommand != msg.data.msg.clawGrip) {
            clawCommand = msg.data.msg.clawGrip;
            count = 0;
        }

        //store pot values
        fromMsg.msg.swerveLeft = analogRead(LEFT_SWERVE_POT);
        fromMsg.msg.swerveRight = analogRead(RIGHT_SWERVE_POT);
        fromMsg.msg.pot0 = analogRead(ARM_POT);
        fromMsg.msg.pot1 = msg.data.msg.flSpeed * (4096.0/20000.0);
        fromMsg.msg.pot2 = msg.data.msg.flSpeed;
        fromMsg.msg.pot3 = 100;

        fromMsg.msg.magic = MESSAGE_MAGIC;
        sendMsg(fromMsg);
    }

}

void setPin(int port, int rand, int pwmValue) {
   pwm.setPWM(port, rand, pwmValue*(4096.0/20000.0)); 
}

/**
 * Attempts to read a new serial message.
 * @return success=false if no message, otherwise a filled msg struct
 */
bool recieveMsg() {

    toMsgAdapter resp;
    if(bytesRead < sizeof(struct toControlMsg)) {
        //resp.success = false;
        return false;
    } else {
        //memcpy(resp.data.structBytes, buffer, sizeof(struct toControlMsg));
        //mySerial.readBytes(resp.data.structBytes, sizeof(struct toControlMsg));
        //resp.success = true;
        bytesRead = 0;
        return true;

    }
}

void sendMsg(toNucAdapter msg) {
  for(int i = 0; i < sizeof(struct toNUCMsg); ++i) {
    mySerial.write(msg.structBytes[i]);
  }
}
