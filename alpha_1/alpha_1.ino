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
int story_rotation = 0;
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
}

void loop()
{

  printDisplay("Print B", "to +135!");
  while (buttonMonitor() != 'B');
  wait();
  lcd.clear();
  dir(135);
  printDisplay("Print B", "to -135!");
  while (buttonMonitor() != 'B');
  wait();
  lcd.clear();
  dir(-135);
}
void wait() {
  lcd.clear();
  for (float i = 2; i > 0; i = i - 0.1) {
    printDisplay(0, 0, i);
    delay(100);
  }
}
void dir(int rotation) {
  char direzione = 'L';
  if ( rotation == NULL) {
    turnSensorUpdate();
    rotation = -(((int32_t)turnAngle >> 16 ) * 360) >> 16;
  }
  if (rotation > 0) {
    direzione = 'L';
  }
  else  if (rotation < 0) {
    direzione = 'R';
  }
  rotation = abs(rotation);
  rotation = ((rotation / 45) * 120);
  mot(SPEED_MAX, direzione);
  if (rotation > 10)
    delay(rotation);
  mot(0, direzione);
}
void mot(int hp, char direzione) {
  hp = constrain(hp, -SPEED_MAX, SPEED_MAX);
  if (direzione == 'L')
    motors.setSpeeds(-hp, hp);
  else
    motors.setSpeeds(hp, -hp);
}
void printDisplay(String line1, String line2) {
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
void printDisplay(int x, int y, String s) {
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
    return 'A';
  if (buttonB.getSingleDebouncedPress())
    return 'B';
  if (buttonC.getSingleDebouncedPress())
    return 'C';
  return 0;
}
