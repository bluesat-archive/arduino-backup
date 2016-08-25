#include <Arduino.h>
#include "libraries/Adafruit_PWMServoDriver/Adafruit_PWMServoDriver.h"
#include "message.h"
#include "pins.h"


toMsgAdapter recieveMsg();
void sendMsg(toNucAdapter msg);
// Initialise the pwm board, note we may need to change the address
// see examples
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup() {
    Serial.begin(19200); //this is the speed specified on line 109 of Bluetongue.cpp I think it is correct
}

void loop() {
    toMsgAdapter msg;
    toNucAdapter fromMsg;

    //TODO: add adc code here
    //TODO: add claw feeedback here

    msg = recieveMsg();
    if (msg.success) {
        //note: we may want to wrap this with our saftey caps like it is on the board
        //I'm not 100% that this set PWM function actually sets a us pulse width, need to double check

        pwm.setPWM(FL_SPEED, 0, msg.data.msg.flSpeed);
        pwm.setPWM(BL_SPEED, 0, msg.data.msg.blSpeed);
        pwm.setPWM(FR_SPEED, 0, msg.data.msg.frSpeed);
        pwm.setPWM(BR_SPEED, 0, msg.data.msg.brSpeed);


        pwm.setPWM(FL_ANG, 0, msg.data.msg.flAng);
        pwm.setPWM(FR_ANG, 0, msg.data.msg.frAng);


        pwm.setPWM(ARM_ROT, 0, msg.data.msg.armRotate);
        pwm.setPWM(ARM_TOP, 0, msg.data.msg.armTop);
        pwm.setPWM(ARM_BOT, 0, msg.data.msg.armBottom);

        //These two + grip are 7.4V
        pwm.setPWM(CLAW_ROT, 0, msg.data.msg.clawRotate);
        pwm.setPWM(LIDAR_TILT, 0, msg.data.msg.lidarTilt);

        // do claw grip stuff here

        //store pot values

        fromMsg.msg.magic = MESSAGE_MAGIC;
        sendMsg(fromMsg);
    }

}


/**
 * Attempts to read a new serial message.
 * @return success=false if no message, otherwise a filled msg struct
 */
toMsgAdapter recieveMsg() {

    toMsgAdapter resp;
    if(Serial.available() < sizeof(struct toControlMsg)) {
        resp.success = false;
        return resp;
    } else {
        Serial.readBytes(resp.data.structBytes, sizeof(struct toControlMsg));
        resp.success = true;
    }
    return resp;
}

void sendMsg(toNucAdapter msg) {
    Serial.write((const uint8_t *)msg.structBytes, sizeof(struct toNUCMsg));
}