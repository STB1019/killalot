//This stuff code make some foundamentals movement
#include<inttypes.h>
#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"
#define SPEED_MAX 400
#define GYRO_NUM 14680064
#define GYRO_DEN 17578125

#define NUM_SENSORS 5
uint16_t lineSensorValues[NUM_SENSORS];
int dirs[] = { -45, 180, 45}; //inverto direzione

Zumo32U4LCD lcd;
Zumo32U4Buzzer buzzer;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
LSM303 compass;
L3G gyro;
Zumo32U4Motors motors;
Zumo32U4Encoders encoders;
void dir(int);
void mot(int32_t, int32_t);
void mot(int32_t);
void printDisplay(int, int, String);
void printDisplay(String, String);
void findWhite();
void setup()
{
  printDisplay("Press A", "to Cal!");
  while (buttonMonitor() != 'A');
  turnSensorSetup();
  delay(500);
  turnSensorReset();
  lineSensors.initFiveSensors();
  printDisplay("Print B", "to Run!");
  while (buttonMonitor() != 'B');
}

void loop()
{
  // Read the gyro to update turnAngle, the estimation of how far
  // the robot has turned, and turnRate, the estimation of how
  // fast it is turning.

  //dir(45);
  printReadingsToSerial(0);
  lineSensors.read(lineSensorValues, QTR_EMITTERS_ON);
  printReadingsToSerial(1);
  mot(50);
  findWhite();
  ledGreen(1);
}
void wait() {
  lcd.clear();
  for (float i = 2; i > 0; i = i - 0.1) {
    printDisplay(0, 0, i);
    delay(100);
  }
}
void dir(int rotation) {
  int32_t verso;
  char buffer[80];
  if ( rotation == NULL) {
    turnSensorUpdate();
    verso = (int32_t)turnAngle;
  }
  else {
    sprintf(buffer, "%4d rotation raw \n", rotation);
    Serial.print(buffer);
    if (rotation > 180) {
      rotation = rotation - 360;
      sprintf(buffer, "%4d rotation over 180 \n", rotation);
    }
    else if (rotation < -180) {
      rotation = rotation + 360;
      sprintf(buffer, "%4d rotation over -180 \n", rotation);
    }
    Serial.print(buffer);
    verso = (int32_t)rotation * 14680064 / 17578125;
    sprintf(buffer, "%4d rotation <-> %" PRIu32 " verso \n", rotation, verso);
    Serial.print(buffer);
  }
  int32_t turnSpeed = 0, a, b;
  a = ((verso >> 16 ) * 360) >> 16;
  b = (((int32_t)turnAngle >> 16) * 360) >> 16;
  printDisplay(0, 0, a );
  printDisplay(4, 0, b );
  while (a != b) {
    turnSensorUpdate();
    turnSpeed = -verso / (turnAngle1 / 56) - turnRate / 20;
    mot(turnSpeed, -turnSpeed);
    a = ((verso >> 16 ) * 360) >> 16;
    b = (((int32_t)turnAngle >> 16) * 360) >> 16;
    printDisplay(0, 0, a );
    printDisplay(4, 0, b );
  }
  mot((int32_t)0);
}
void mot(int all) {
  mot((int32_t)all);
}
void mot(int32_t all) {
  all = constrain(all, -SPEED_MAX, SPEED_MAX);
  motors.setSpeeds(all, all);
  printDisplay(1, 1, all);
  printDisplay(4, 1, "|");
  printDisplay(5, 1, all);
}
void mot(int32_t left, int32_t right) {
  left = constrain(left, -SPEED_MAX, SPEED_MAX);
  right = constrain(right, -SPEED_MAX, SPEED_MAX);
  motors.setSpeeds(left, right);
  printDisplay(0, 1, left);
  printDisplay(4, 1, "|");
  printDisplay(5, 1, right);
}
void printDisplay(String line1, String line2) {
  lcd.clear();
  printDisplay(0, 0, line1);
  printDisplay(0, 1, line2);
}
void printDisplay(int x, int y, float value) {
  x = constrain(x, -8, 8);
  y = constrain(y, -8, 8);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(value);
}
void printDisplay(int x, int y, int32_t value) {
  x = constrain(x, -8, 8);
  y = constrain(y, -8, 8);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(value);
}
void printDisplay(int x, int y, String s) {
  x = constrain(x, -8, 8);
  y = constrain(y, -8, 8);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(s);
}

char buttonMonitor()
{
  if (buttonA.getSingleDebouncedPress())
  {
    return 'A';
  }

  if (buttonB.getSingleDebouncedPress())
  {
    return 'B';
  }

  if (buttonC.getSingleDebouncedPress())
  {
    return 'C';
  }

  return 0;
}

/*Detect Line*/
void printReadingsToSerial(int a)
{
  char buffer[80];
  sprintf(buffer, "%2d -> %4d %4d %4d %4d %4d\n",
          a,
          lineSensorValues[0],
          lineSensorValues[1],
          lineSensorValues[2],
          lineSensorValues[3],
          lineSensorValues[4]
         );
  Serial.print(buffer);
}

void findWhite() {
  for (int i = 0; i < 5; i++) {
    printReadingsToSerial(2);
    lineSensors.read(lineSensorValues, QTR_EMITTERS_ON);
    if (lineSensorValues[i] <= 600) {
      ledGreen(0);
      lcd.clear();
      printReadingsToSerial(3);
      dir(dirs[i]);
      mot(0);
      delay(1000);
    }
  }
}
