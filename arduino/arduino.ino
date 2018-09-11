
#include <Wire.h>

#include <Arduino.h>
#include "Adafruit_PWMServoDriver.h"
#include "message.h"
#include "pins.h"
#include <SoftwareSerial.h>
#include <Servo.h>

#define LIDAR_SERVO_PIN 2

SoftwareSerial mySerial(10, 11); // RX, TX
Servo lidarServo;

bool recieveMsg();
void sendMsg(toNucAdapter msg);
void clawFeedbackIteration(toNucAdapter *fromMsgPtr);

Servo myservo; // create servo object to control the lidar servo
void lidarRotate(float tilt);
// Initialise the pwm board, note we may need to change the address
// see examples
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

char buffer[sizeof(toControlMsg)];
char bytesRead;


/********** COPY FROM CLAW FEEDBACK ******************/

#define ERROR_NUMBER 70
//#define CLAWPIN 0 // depends on which pinit's attached to on i2c to pwm module

toNucAdapter fromMsg;
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
    Serial.begin(9600);
    // Serial.begin(19200);
    myservo.writeMicroseconds(1500); //set initial servo position if desired
    myservo.attach(LIDAR_SERVO_PIN); //the pin for the servo control 

    Serial.println("servo-test-22-dual-input"); // so I can keep track of what is loaded
}
bool foundFirst = false;
toMsgAdapter msg;

/**
 * Do claw feedback stuff
 * @param fromMsgPtr incoming message
 */
void clawFeedbackIteration(toNucAdapter *fromMsgPtr) {

   //Calculating Claw Actual
   float sensorValue = 0;                    // value read from the pot
   sensorValue = analogRead(CLAW_FEEDBACK);
   sensorValue = sensorValue * 3.30609 + 430; // calibration numbers
   int clawActual = (int)sensorValue;
   fromMsgPtr->msg.gripEffort = clawCommand;
   fromMsgPtr->msg.clawActual = clawActual;
   setPin(CLAW_GRIP, 0, clawCommand);
}

/**
 * Main loop
 */
void loop() {

    byte MAGIC[2] = {0x55, 0xAA};
    
    while(mySerial.available() > 0) {
        char val = mySerial.read();
        if(val == MAGIC[0]) {
          bytesRead = 0;
          foundFirst = true;
        }

        if(foundFirst) {
          msg.data.structBytes[++bytesRead] = val;
          if(bytesRead >=  sizeof(struct toControlMsg))  {
            break;
          }
        }
    }

    clawFeedbackIteration(&fromMsg);

    //clawFeedbackError2
    msg.success = recieveMsg();

    if (bytesRead > 0) {
        
        //note: we may want to wrap this with our saftey caps like it is on the board

        setPin(FL_SPEED, 0, msg.data.msg.flSpeed);
        setPin(BL_SPEED, 0, msg.data.msg.blSpeed);
        setPin(FR_SPEED, 0, msg.data.msg.frSpeed);
        setPin(BR_SPEED, 0, msg.data.msg.brSpeed);


        setPin(FL_ANG, 0, msg.data.msg.flAng);
        setPin(FR_ANG, 0, msg.data.msg.frAng);


        setPin(ARM_ROT, 0, msg.data.msg.armRotate);
        setPin(ARM_TOP, 0, msg.data.msg.armTop);
        setPin(ARM_BOT, 0, msg.data.msg.armBottom);

        //These two + grip are 7.4V
        setPin(CLAW_ROT, 0, msg.data.msg.clawRotate);

        // setPin(LIDAR_TILT, 0, msg.data.msg.lidarTilt); // Change this shit
        lidarRotate(msg.data.msg.lidarTilt);
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

void lidarRotate(float tilt) {
    // setPin(LIDAR_TILT, 0, msg.data.msg.lidarTilt); // Change this shit
    float n = tilt * 57296 / 1000; // degree to rad

    // auto select appropriate value, copied from someone else's code.
    if(n >= 500) {
        Serial.print("writing Microseconds: ");
        Serial.println(n);
        myservo.writeMicroseconds(n);
    } else {
        Serial.print("writing Angle: ");
        Serial.println(n);
        myservo.write(n);
    }
}

/**
 * Sets a PWM value for a pin
 * @param port the port on the pwm board to set (off by one)
 * @param rand 0??
 * @param pwmValue the value to set
 */
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
        return false;
    } else {
        bytesRead = 0;
        return true;
    }
}

/**
 * Write out the message back to the nuc
 * @param msg the message to write out
 */
void sendMsg(toNucAdapter msg) {
  for(int i = 0; i < sizeof(struct toNUCMsg); ++i) {
    mySerial.write(msg.structBytes[i]);
  }
}
