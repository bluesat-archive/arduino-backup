#include <Wire.h>

#include <Arduino.h>
#include "Adafruit_PWMServoDriver.h"
#include "message.h"
#include "pins.h"


bool recieveMsg();
void sendMsg(toNucAdapter msg);
// Initialise the pwm board, note we may need to change the address
// see examples
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

char buffer[sizeof(toControlMsg)];
char bytesRead;

void setup() {
    Serial.begin(19200); //this is the speed specified on line 109 of Bluetongue.cpp I think it is correct
    bytesRead = 0;
    pwm.begin();
  
    pwm.setPWMFreq(52);
}
bool foundFirst = false;
toMsgAdapter msg;
void loop() {
    
    toNucAdapter fromMsg;
    
    
    byte MAGIC[2] = {0x55, 0xAA};
    
    while(Serial.available() > 0) {
        char val = Serial.read();
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
    //TODO: add claw feeedback here
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
        //Serial.readBytes(resp.data.structBytes, sizeof(struct toControlMsg));
        //resp.success = true;
        bytesRead = 0;
        return true;

    }
}

void sendMsg(toNucAdapter msg) {
  for(int i = 0; i < sizeof(struct toNUCMsg); ++i) {
    Serial.write(msg.structBytes[i]);
  }
}
