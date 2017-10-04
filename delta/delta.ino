//This stuff code make some foundamentals movement
#include<inttypes.h>
#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"
#define SPEED_MAX 400
#define GYRO_NUM 14680064
#define GYRO_DEN 17578125
#define PROX_TRESHOLD 4
#define SIZE_PROX 3 // 3 fisso!
#define ROUND_TRESHOLD 450 //al max 360*# di giri

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
int story_rotation = 0;
int round_counter = 0;
void dir(int);
void mot(int32_t, int32_t);
void mot(int32_t);
void printDisplay(int, int, String);
void printDisplay(String, String);
void scan();
void setup()
{
  Serial.begin(9600);
  printDisplay("Press A", "to Cal!");
  while (buttonMonitor() != 'A');
  turnSensorSetup();
  delay(500);
  turnSensorReset();
  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();
  printDisplay("Press B", "to Fight!");
  while (buttonMonitor() != 'B');
  wait();
  dir(135);
}

void loop()
{
  delay(200);
  lcd.clear();
  scan();

}
void wait() {
  lcd.clear();
  for (float i = 5; i > 0; i = i - 0.1) {
    printDisplay(0, 0, i);
    delay(100);
  }
}
/*Direction*/
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
    sprintf(buffer, "-> verso= %" PRIu32 " rotation %4d story_rotation %4d \n", verso, rotation , story_rotation);
    Serial.print(buffer);
    rotation = rotation + story_rotation; // TODO

    if (rotation == 0)
      rotation = -story_rotation;
    story_rotation = rotation;
    //    if (rotation > 180)
    //      rotation = rotation - 360;
    //    else if (rotation < -180)
    //      rotation = rotation + 360;
    verso = rotation * turnAngle1;
    sprintf(buffer, "After multiply %" PRIu32 "= %4d * %" PRIu32 " \n", verso, rotation , turnAngle1);
    Serial.print(buffer);
  }
  dteta = (((int32_t)verso >> 16 ) * 360) >> 16;
  teta = (((int32_t)turnAngle >> 16) * 360) >> 16;
  printDisplay(0, 0, (int32_t)dteta );
  printDisplay(3, 0, (int32_t)teta );
  printDisplay(5, 0, (int32_t)(((int32_t)(story_rotation * turnAngle1) >> 16 ) * 360) >> 16 );
  while (dteta > teta) {
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
/*Motor*/
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
}

/*STATE scan*/
void scan() {
  mot(-100, 100);
  delay(100);
  ledRed(0);
  ledGreen(0);
  ledYellow(0);
  proxSensors.read();
  switch (look()) {
    case 'W':
      ledGreen(1);
      mot(100);
      delay(100);
      break;
    case 'A':
      ledRed(1);
      mot(-100, 100);
      delay(100);
      break;
    case 'D':
      ledYellow(1);
      mot(100, -100);
      delay(100);
      break;
    case 'S': // stop
    default:
      round_counter += 60;
      if (round_counter >= ROUND_TRESHOLD) {
        round_counter = 0;
        mot(50);
        delay(200);
      }
      delay(10);
      break;
  }
}
/*look function*/
char look() {
  if (proxSensors.countsFrontWithLeftLeds() > PROX_TRESHOLD || proxSensors.countsFrontWithRightLeds() > PROX_TRESHOLD)
    return 'W';
  if (proxSensors.countsLeftWithLeftLeds() > PROX_TRESHOLD || proxSensors.countsLeftWithRightLeds() > PROX_TRESHOLD - 1)
    return 'A';
  if (proxSensors.countsRightWithLeftLeds() > PROX_TRESHOLD - 1 || proxSensors.countsRightWithRightLeds() > PROX_TRESHOLD)
    return 'D';
  return 'S';
}
void printDisplay(String line1, String line2) {
  lcd.clear();
  printDisplay(0, 0, line1);
  printDisplay(0, 1, line2);
}
void printDisplay(int line1, int line2) {
  lcd.clear();
  printDisplay(0, 0, line1);
  printDisplay(0, 1, line2);
}
void printDisplay(int x, int y, float value) {
  x = constrain(x, 0, 8);
  y = constrain(y, 0, 1);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(value);
}
void printDisplay(int x, int y, int32_t value) {
  x = constrain(x, 0, 8);
  y = constrain(y, 0, 1);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(value);
}
void printDisplay(int x, int y, String s) {
  x = constrain(x, 0, 8);
  y = constrain(y, 0, 1);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(s);
}
void printDisplay(int x, int y, int s) {
  x = constrain(x, 0, 8);
  y = constrain(y, 0, 1);
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
