#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"

#define SPEED_MAX 400
#define GYRO_NUM 14680064
#define GYRO_DEN 17578125
#define PROX_TRESHOLD 4
#define SIZE_PROX 3 // 3 fisso!
#define ROUND_TRESHOLD 450 //al max 360*# di giri

// Soglia bordo bianco

#define LINE_SENSOR_THRESHOLD 700


//// Tempo
// The amount of time to spend backing up after detecting a border, in milliseconds.
#define reverseTime  200
// The minimum amount of time to spend scanning for nearby opponents, in milliseconds.
#define scanTimeMin  200
// The maximum amount of time to spend scanning for nearby opponents, in milliseconds.
#define scanTimeMax  2100
// Tempo attesa iniziale
#define waitTime  2000
/* If the robot has been driving forward for this amount of time, in milliseconds,
   without reaching a border, the robot decides that it must be pushing on
   another robot and this is a stalemate, so it increases its motor speed.)
*/
#define stalemateTime  4000

Zumo32U4LCD lcd;
Zumo32U4ButtonA buttonA, buttonB, buttonC;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
L3G gyro;

unsigned int lineSensorValues[3];
int round_counter=0;
void mot(int, int);
void mot(int);
void mot(int, char);
int story_rotation = 0;
char buffer[80];
// The time, in milliseconds, that we entered the current top-level state.
uint16_t stateStartTime;
// The time, in milliseconds, that the LCD was last updated.
uint16_t displayTime;

//// Lista stati
enum State
{
  StateSeek,
  StateScan,
  StatePausing,
  StateWaiting,
  StateScanning,
  StateDriving,
  StateBacking,
  StateWhite,
};

State state = StatePausing;

bool justChangedState;


enum Direction {
  DirectionLeft,
  DirectionRight,
};

Direction scanDir = DirectionLeft;


void setup() {
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
  changeState(StateScan);
}


void loop()
{
  switch (state) {
    case StateScan:
    default:
      scanState();
      break;
    case StateSeek:
      if (seekState())
        ramState();
      else
        whiteState();
      break;
    case StateBacking:
      mot('S', 250);
      delay(100);
    case StateWhite:
      whiteState();
      break;
  }
}


int16_t timeInThisState() {
  return (uint16_t)(millis() - stateStartTime);
}

void changeState(uint8_t newState) {
  state = (State)newState;
  justChangedState = true;
  stateStartTime = millis();
  ledRed(0);
  ledYellow(0);
  ledGreen(0);
  lcd.clear();
}

void scanState() {
  /*
     Ho Bianco? Se sì interrompi e setta lo stato su White
  */
  if (findWhite())
    return;
  mot(-100, 100);
  proxSensors.read();
  switch (look()) {
    case 'W':
      ledGreen(1);
      mot(250,'W');
      break;
    case 'A':
      ledRed(1);
      mot(200,'S');
      mot(250,'A');
      break;
    case 'D':
      ledYellow(1);
      mot(200,'S');
      mot(250,'D');
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

boolean seekState() {
  return false;
  return true;
}

void whiteState() {
  switch (scanDir) {
    case DirectionRight:
      mot('R', 250);
      break;
    case DirectionLeft:
      mot('L', 250);
      break;
  }
}
/**
   Verifica se c'è del Bianco rilevato dai sensori. Predispone la mossa successiva
*/
bool findWhite() {
  lineSensors.read(lineSensorValues);
  if (lineSensorValues[0] < LINE_SENSOR_THRESHOLD)
  {
    scanDir = DirectionRight;
    changeState(StateBacking);
    return true;
  }
  if (lineSensorValues[2] < LINE_SENSOR_THRESHOLD)
  {
    scanDir = DirectionLeft;
    changeState(StateBacking);
    return true;
  }
  return false;
}
void ramState() {}
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
    rotation = rotation + story_rotation;
    if (rotation == 0)
      rotation = -story_rotation;
    story_rotation = rotation;
    //    if (rotation > 180)
    //      rotation = rotation - 360;
    //    else if (rotation < -180)
    //      rotation = rotation + 360;
    verso = rotation * turnAngle1;
  }
  dteta = (((int32_t)verso >> 16 ) * 360) >> 16;
  teta = (((int32_t)turnAngle >> 16) * 360) >> 16;
  while (dteta > teta) {
    turnSensorUpdate();
    turnSpeed = (int32_t)verso / (turnAngle1 / 56) - turnRate / 20;
    mot((int) - turnSpeed, (int)turnSpeed);
    dteta = (((int32_t)verso >> 16 ) * 360) >> 16;
    teta = (((int32_t)turnAngle >> 16) * 360) >> 16;
  }
  mot((int32_t)0);
  changeState(StateSeek);
}

void mot(int all) {
  all = constrain(all, -SPEED_MAX, SPEED_MAX);
  motors.setSpeeds(all, all);
}
void mot(int left, int right) {
  left = constrain(left, -SPEED_MAX, SPEED_MAX);
  right = constrain(right, -SPEED_MAX, SPEED_MAX);
  motors.setSpeeds(left, right);
}
void mot(int hp, char dir) {
  hp = constrain(hp, -SPEED_MAX, SPEED_MAX);
  switch (dir) {
    case'L':
    case'l':
    case'A':
    case'a':
      motors.setSpeeds(-hp, hp);
      break;
    case'R':
    case'r':
    case'D':
    case'd':
      motors.setSpeeds(hp, -hp);
      break;
    case'S':
    case's':
      motors.setSpeeds(-hp, -hp);
      break;
    case'W':
    case'w':
    default:
      motors.setSpeeds(hp, hp);
      break;
  }
  delay(100);
}
void printDisplay(String line1, String line2) {
  lcd.clear();
  printDisplay(0, 0, line1);
  printDisplay(0, 1, line2);

}
void printDisplay(int x, int y, String value) {
  x = constrain(x, 0, 8);
  y = constrain(y, 0, 1);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(value);
}
void printDisplay(int x, int y, float value) {
  x = constrain(x, 0, 8);
  y = constrain(y, 0, 1);
  lcd.gotoXY(x, y);
  lcd.print(F("    "));
  lcd.gotoXY(x, y);
  lcd.print(value);
}
void printSerial() {
  sprintf(buffer, "%d %d %d %d %d %d - %4d %4d %4d %4d %4d\n",
          proxSensors.countsLeftWithLeftLeds(),
          proxSensors.countsLeftWithRightLeds(),
          proxSensors.countsFrontWithLeftLeds(),
          proxSensors.countsFrontWithRightLeds(),
          proxSensors.countsRightWithLeftLeds(),
          proxSensors.countsRightWithRightLeds(),
          lineSensorValues[0],
          lineSensorValues[1],
          lineSensorValues[2],
          lineSensorValues[3],
          lineSensorValues[4]
         );
  Serial.print(buffer);
  sprintf(buffer, "", "");
}
char buttonMonitor() {
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
void wait() {
  lcd.clear();
  for (float i = 5; i > 0; i = i - 0.1) {
    printDisplay(0, 0, i);
    delay(100);
  }
}
