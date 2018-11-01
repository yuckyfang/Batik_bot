/***************************************************
  This example reads HighTemperature Sensor.

  Created 2016-1-13
  By berinie Chen <bernie.chen@dfrobot.com>

  GNU Lesser General Public License.
  See <http://www.gnu.org/licenses/> for details.
  All above must be included in any redistribution
 ****************************************************/

/***********Notice and Trouble shooting***************
  1.Connection and Diagram can be found here  https://www.dfrobot.com/wiki/index.php?title=HighTemperatureSensor_SKU:SEN0198
  2.This code is tested on Arduino Uno.
 ****************************************************/
#include<DFRobotHighTemperatureSensor.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include <AccelStepper.h>
#define meltingPoint 53

const float voltageRef = 5.000; //Set reference voltage,you need test your IOREF voltage.
//const float voltageRef = 3.300;
int HighTemperaturePin = A0;  //Setting pin
DFRobotHighTemperature PT100 = DFRobotHighTemperature(voltageRef); //Define an PT100 object
int currentTemperature = 0;


//
////------------------for stepper motor---------------------//

// Create the motor shield object with the default I2C address
//Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
Adafruit_MotorShield AFMS_level2 = Adafruit_MotorShield(0x61);

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4), port #1 is M1 and M2
Adafruit_StepperMotor *myHeater = AFMS_level2.getStepper(200, 1); //for the heating resistor
Adafruit_StepperMotor *myMotor = AFMS_level2.getStepper(200, 2);
////------------------for stepper motor---------------------//
void forwardstepE() {
  myMotor->onestep(FORWARD, DOUBLE);
}
void backwardstepE()  {
  myMotor->onestep(BACKWARD, DOUBLE);
}
AccelStepper Estepper(forwardstepE, backwardstepE); // use functions to step  for syringe
void initializeHeater() {
  currentTemperature = PT100.readTemperature(HighTemperaturePin);
  int curtemp = currentTemperature;
  while (currentTemperature < meltingPoint) {
    if(curtemp > 51){
      break;
    }
    currentTemperature = PT100.readTemperature(HighTemperaturePin);  //Get temperature
    Serial.print("temperature:  ");
    Serial.print(currentTemperature);
    Serial.println("  ^C");
    myHeater->setSpeed(50);
    myHeater->onestep(FORWARD, DOUBLE);
  }
  Serial.println("delaying...");
  long waittime = 300000;
  long actual = waittime*(53 - curtemp)/(53 - 30);
  delay(actual);  //5 min
  Serial.println("delay done");
  myHeater -> release();
}

void setup(void) {
  Serial.begin(9600);
  //AFMS.begin(1600);  // OR with a different frequency, say 1KHz
  AFMS_level2.begin(1600);
  Estepper.setMaxSpeed(250);
  initializeHeater();
}

void loop(void) {
  currentTemperature = PT100.readTemperature(HighTemperaturePin);  //Get temperature
  Serial.print("temperature is:  ");
  Serial.print(currentTemperature);
  Serial.println("  ^C");
  while (currentTemperature <= 53) {
    Serial.println("Temperature lower than melting point. Reheating...");
    currentTemperature = PT100.readTemperature(HighTemperaturePin);  //Get temperature
    Serial.print("temperature is:  ");
    Serial.print(currentTemperature);
    Serial.println("  ^C");
    myHeater->onestep(FORWARD, DOUBLE);
  }
  if (currentTemperature > 53) {
    Serial.println("Not heating");
    myHeater -> release();
  }
  Estepper.move(60); // Move 2.5 rotations or 0.5cm down
  // Use non-blocked command in while-loop instead of blocked command
  // because blocked command does not work for some reason...
  Serial.println("Pressing syringe down");
  while (Estepper.currentPosition() != Estepper.targetPosition()) {
    Estepper.setSpeed(50);
    Estepper.runSpeedToPosition();
  }
  //release the stepper motor to save the current
  delay(10000);
  myMotor -> release();
  delay(1000); //just here to slow down the output so it is easier to read

}
