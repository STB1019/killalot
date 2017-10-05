
#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"
//#define FLIP_LEFT
//#define FLIP_RIGHT
#define SPEED_MAX 400
#define SIZE_LINE_SENSOR 3
#define TRESHOLD_LINE_SENSOR 700
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
unsigned int lineSensorValues[SIZE_LINE_SENSOR];
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


void setup(){
  #ifdef FLIP_LEFT
  motors.flipLeftMotor(true);
  #endif
  #ifdef FLIP_RIGHT
  motors.flipRightMotor(true); 
  #endif
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

void loop(){
  changeState();
  switch (state) {
    case state_White:
      StateWhite();
      break;
    case state_Scan:
    default:
      StateScan();
      break;
    case state_Seek:
      StateSeek();
      break;
    case state_Ram:
      StateRam();
      break;
  }
  delay(100);
}

uint16_t timeInThisState(){
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
  proxSensors.read();
  uint8_t sum = proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds();
  if (proxSensors.countsRightWithRightLeds() >= 5)
    dir(-60);
  else if (proxSensors.countsLeftWithLeftLeds() >= 5)
    dir(60);
  else if (sum >= 4 || timeInThisState() > STALEMATETIME)
    StateRam();
}
bool isProximity() {
  proxSensors.read();
  uint8_t sum = proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds();
  int8_t diff = proxSensors.countsFrontWithRightLeds() - proxSensors.countsFrontWithLeftLeds();
  if (
    proxSensors.countsRightWithRightLeds() >= 5 ||
    proxSensors.countsLeftWithLeftLeds() >= 5 ||
    sum >= 4 ||
    timeInThisState() > STALEMATETIME) {
    modState(state_Seek);
    return true;
  }
  return false;
}
/*Attuatore per lo stato White*/
void StateWhite() {
  //TODO
  ledGreen(0);
  delay(100);
  ledGreen(1);
  delay(100);
  ledGreen(0);
  delay(100);
  ledGreen(1);
  modState(state_Scan);
}
bool isWhite() {
  return false;
  lineSensors.read(lineSensorValues);
  for (int i = 0; i < SIZE_LINE_SENSOR; i++)
    if (lineSensorValues[i] < TRESHOLD_LINE_SENSOR) {
      modState(state_White);
      return true;
    }
  return false;
}
/*Attuatore per lo stato Ram*/
void StateRam() {
  //TODO
  ledRed(0);
  delay(100);
  ledRed(1);
  delay(100);
  ledRed(0);
  delay(100);
  ledRed(1);
  mot(SPEED_MAX,dir_Forward);
  modState(state_Scan);
}
/**
    Attesa 5 secondi
*/
void wait() {
  lcd.clear();
  for (float i = 5; i > 0; i = i - 0.1) {
    printDisplay(0, 0, i);
    delay(100);
  }
}
/*
   Set relative rotation in degrees [-360:1:360], not absolute!
*/
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
/**
   hp= Horse Power
   dir= direction
*/
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

char buttonMonitor() {
  if (buttonA.getSingleDebouncedPress())
    return 'A';
  if (buttonB.getSingleDebouncedPress())
    return 'B';
  if (buttonC.getSingleDebouncedPress())
    return 'C';
  return 0;
}
void modState(State s) {
  state = s;
  printDisplay("State ", state);
  delay(250);
  //while (buttonMonitor() != 'A');

}


