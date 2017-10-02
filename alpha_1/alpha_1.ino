//This stuff code make some foundamentals movement
#include<inttypes.h>
#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"
#define SPEED_MAX 400
#define GYRO_NUM 14680064
#define GYRO_DEN 17578125

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
int story_rotation=0;
void dir(int);
void mot(int32_t, int32_t);
void mot(int32_t);
void printDisplay(int, int, String);
void printDisplay(String, String);
void setup()
{
  Serial.begin(9600);
  printDisplay("Print A", "to Cal!");
  while (buttonMonitor() != 'A');
  turnSensorSetup();
  delay(500);
  turnSensorReset();
  printDisplay("Print B", "to +135!");
  while (buttonMonitor() != 'B');
  wait();
  lcd.clear();
  dir(135);
  while (buttonMonitor() != 'B');
  ledGreen(1);
  printDisplay("Print B", "to -135!");
  while (buttonMonitor() != 'B');
  wait();
  lcd.clear();
  dir(-135);
  while (buttonMonitor() != 'B');
  ledGreen(1);
  printDisplay("Print B", "to Fight!");
  while (buttonMonitor() != 'B');
  wait();
  lcd.clear();
  dir(90);
  while (buttonMonitor() != 'B');
  ledGreen(1);
  printDisplay("Print B", "to Fight!");
  while (buttonMonitor() != 'B');
  wait();
  lcd.clear();
  dir(90);
  while (buttonMonitor() != 'B');
  ledGreen(1);
  printDisplay("Print B", "to Fight!");
  while (buttonMonitor() != 'B');
  wait();
  lcd.clear();
  dir(-135);
  while (buttonMonitor() != 'B');
  ledGreen(1);
  printDisplay("Print B", "to Fight!");
  while (buttonMonitor() != 'B');
}

void loop()
{
  // Read the gyro to update turnAngle, the estimation of how far
  // the robot has turned, and turnRate, the estimation of how
  // fast it is turning.

  dir(NULL);

}
void wait() {
  lcd.clear();
  for (float i = 2; i > 0; i = i - 0.1) {
    printDisplay(0, 0, i);
    delay(100);
  }
}
void dir(int rotation) {
  int64_t verso;
  int teta = (((int32_t)turnAngle >> 16) * 360 >> 16);
  int32_t turnSpeed = 0;
  int dteta = 0;
  char buffer[80];
  if ( rotation == NULL) {
    turnSensorUpdate();
    verso = (int64_t)360;
  }
  else {
    ledGreen(0);
    sprintf(buffer, "%4d rotation + %4d=%4d\n", rotation, teta, rotation + teta);
    Serial.print(buffer);
    rotation += story_rotation; // TODO
    story_rotation=rotation;
    if (rotation > 180) {
      rotation = rotation - 360;
      sprintf(buffer, "%4d rotation over 180 \n", rotation);
    }
    else if (rotation < -180) {
      rotation = rotation + 360;
      sprintf(buffer, "%4d rotation over -180 \n", rotation);
    }
    Serial.print(buffer);
    verso = rotation * turnAngle1;
    sprintf(buffer, "%" PRIu32 "= %4d * %" PRIu32 " \n", verso, rotation , turnAngle1);
    Serial.print(buffer);
  }
  dteta = (((int32_t)verso >> 16 ) * 360) >> 16;
  teta = (((int32_t)turnAngle >> 16) * 360) >> 16;
  printDisplay(0, 0, (int32_t)dteta );
  printDisplay(4, 0, (int32_t)teta );
  while (dteta != teta) {
    turnSensorUpdate();
    turnSpeed = (int32_t)verso / (turnAngle1 / 56) - turnRate / 20;
    mot(-turnSpeed, turnSpeed);
    dteta = (((int32_t)verso >> 16 ) * 360) >> 16;
    teta = (((int32_t)turnAngle >> 16) * 360) >> 16;
    printDisplay(0, 0, (int32_t)dteta );
    printDisplay(4, 0, (int32_t)teta );
  }
  mot((int32_t)0);
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
