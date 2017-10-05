
#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"
#define SPEED_MAX 400
#define GYRO_NUM 14680064
#define GYRO_DEN 17578125
#define STALEMATETIME 4000

Zumo32U4LCD lcd;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
L3G gyro;
Zumo32U4Motors motors;
int story_rotation = 0;
uint16_t stateStartTime;
enum State {
  state_Scan,
  state_Seek,
  state_Ram,
  state_White,
};
State state = state_Scan;
enum Direction
{
  dir_Left,
  dir_Right,
  dir_Forward,
  dir_Back,
  dir_Stop,
};

char buttonMonitor();
void printDisplay(int , int , String );
void printDisplay(String, String);
void printDisplay(int , int , float);

void mot(int , Direction);
void dir(int);
void wait();
void StateScan();
void changeState();
void StateWhite();
bool isWhite();
void StateScan();
void StateSeek();
bool isProximity();
void StateRam();


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
  mot(dir_Stop, 0);
  wait();
  dir(135);
}

void loop()
{
  StateScan();
  delay(100);
  //  changeState();
  //  switch (state) {
  //    case state_White:
  //      StateWhite();
  //      break;
  //    case state_Scan:
  //    default:
  //      StateScan();
  //      break;
  //    case state_Seek:
  //      StateSeek();
  //      break;
  //    case state_Ram:
  //      StateRam();
  //      break;
  //  }
  delay(100);
}

uint16_t timeInThisState()
{
  return (uint16_t)(millis() - stateStartTime);
}
/*Legge sensori: in base a quel che trova applica uno stato*/
void changeState() {
  State neo_state;
  if (isWhite())
    neo_state = state_White;
  else if (isProximity())
    neo_state = state_Seek;
  else
    neo_state = state_Scan;
  if (neo_state != state) {
    neo_state = state;
    stateStartTime = millis();
  }

}
/*Attuatore per lo stato Scan*/
void StateScan() {
  bool sensorSignal = true;
  dir(60);
  story_rotation += 60;
  printDisplay(0, 0, (float)story_rotation);
  if (story_rotation >= 450)
    story_rotation = -2;
  if (isWhite() || isProximity())
    changeState();
  while ((story_rotation < 0) && sensorSignal) {
    mot(250, dir_Forward);
    delay(100);
    story_rotation += 1;
    if ((!isWhite()) || (!isProximity()))
      sensorSignal = false;
  }
  if (!sensorSignal)
    changeState();
}
/*Attuatore per lo stato Seek*/
void StateSeek() {
}
bool isProximity() {
  proxSensors.read();
  uint8_t sum = proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds();
  int8_t diff = proxSensors.countsFrontWithRightLeds() - proxSensors.countsFrontWithLeftLeds();
  printDisplay("Test", sum);
  delay(100);
  ledRed(0);
  delay(100);
  ledRed(1);
  delay(100);
  ledRed(0);
  delay(100);
  ledRed(1);
  if (proxSensors.countsRightWithRightLeds() >= 5 || proxSensors.countsLeftWithLeftLeds() >= 5 || sum >= 4 || timeInThisState() > STALEMATETIME)
    return true;
  return false;
}
/*Attuatore per lo stato White*/
void StateWhite() {

}
bool isWhite() {
  delay(100);
  ledYellow(0);
  delay(100);
  ledYellow(1);
  delay(100);
  ledYellow(0);
  delay(100);
  ledYellow(1);
  return false;
}
/*Attuatore per lo stato Ram*/
void StateRam() {
}
void wait() {
  lcd.clear();
  for (float i = 5; i > 0; i = i - 0.1) {
    printDisplay(0, 0, i);
    delay(100);
  }
}
void dir(int rotation) {
  Direction dir = dir_Left;
  if ( rotation == NULL) {
    turnSensorUpdate();
    rotation = -(((int32_t)turnAngle >> 16 ) * 360) >> 16;
  }
  if (rotation > 0)
    dir = dir_Left;
  else  if (rotation < 0)
    dir = dir_Right;
  rotation = abs(rotation);
  rotation = ((rotation / 45) * 120);
  mot(SPEED_MAX, dir);
  if (rotation > 10)
    delay(rotation);
  mot(0, dir);
}
void mot(int hp, Direction dir) {
  hp = constrain(hp, -SPEED_MAX, SPEED_MAX);
  switch (dir) {
    case dir_Left:
      motors.setSpeeds(-hp, hp);
      break;
    case dir_Forward:
      motors.setSpeeds(hp, hp);
      break;
    case dir_Right:
      motors.setSpeeds(hp, -hp);
      break;
    case dir_Back:
      motors.setSpeeds(-hp, -hp);
      break;
    default:
    case dir_Stop:
      motors.setSpeeds(0, 0);
      break;
  }
}
void printDisplay(String line1, String line2) {
  lcd.clear();
  printDisplay(0, 0, line1);
  printDisplay(0, 1, line2);
}
void printDisplay(String line1, float line2) {
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


