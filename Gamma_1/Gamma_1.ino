/* This example shows how you might use the Zumo 32U4 in a robot
  sumo competition.

  It uses the line sensors to detect the white border of the sumo
  ring so it can avoid driving out of the ring (similar to the
  BorderDetect example).  It also uses the Zumo 32U4's proximity
  sensors to scan for nearby opponents and drive towards them.

  For this code to work, jumpers on the front sensor array
  must be installed in order to connect pin 4 to RGT and connect
  pin 20 to LFT.

  This code was tested on a Zumo 32U4 with 75:1 HP micro metal
  gearmotors. */

#include <Wire.h>
#include <Zumo32U4.h>
#include "TurnSensor.h"
#define SPEED_MAX 400

Zumo32U4LCD lcd;
Zumo32U4ButtonA buttonA;
Zumo32U4Buzzer buzzer;
Zumo32U4Motors motors;
Zumo32U4LineSensors lineSensors;
Zumo32U4ProximitySensors proxSensors;
L3G gyro;

unsigned int lineSensorValues[3];
void mot(int32_t, int32_t);
void mot(int32_t);
int story_rotation = 0;
char buffer[80];

// Soglia bordo bianco
const uint16_t lineSensorThreshold = 700;

//// Velocità
// Velocità back
const uint16_t reverseSpeed = 50;
// Velocità turn
const uint16_t turnSpeed = 50;
// Velocità move
const uint16_t forwardSpeed = 100;
/* Velocità virata (These two variables specify the speeds to apply to the motors when veering
  left or veering right.  While the robot is driving forward, it uses its proximity sensors to
  scan for objects ahead of it and tries to veer towards them.)
*/
const uint16_t veerSpeedLow = -100;
const uint16_t veerSpeedHigh = 100;
/* Velocità ram (The speed that the robot drives when it detects an opponent in front of it,
  either with the proximity sensors or by noticing that it is caught in a stalemate
  (driving forward for several seconds without reaching a border).  400 is full speed.)
*/
const uint16_t rammingSpeed = 100;

//// Tempo
// The amount of time to spend backing up after detecting a border, in milliseconds.
const uint16_t reverseTime = 200;
// The minimum amount of time to spend scanning for nearby opponents, in milliseconds.
const uint16_t scanTimeMin = 200;
// The maximum amount of time to spend scanning for nearby opponents, in milliseconds.
const uint16_t scanTimeMax = 2100;
// Tempo attesa iniziale
const uint16_t waitTime = 2000;
/* If the robot has been driving forward for this amount of time, in milliseconds,
  without reaching a border, the robot decides that it must be pushing on another
  robot and this is a stalemate, so it increases its motor speed.)
*/
const uint16_t stalemateTime = 4000;
// The time, in milliseconds, that we entered the current top-level state.
uint16_t stateStartTime;
// The time, in milliseconds, that the LCD was last updated.
uint16_t displayTime;

//// Lista stati
enum State
{
  StateInit,
  StateSeek,
  StatePausing,
  StateWaiting,
  StateScanning,
  StateDriving,
  StateBacking,
};

State state = StatePausing;

enum Direction
{
  DirectionLeft,
  DirectionRight,
  DirectionForward,
};

// scanDir is the direction the robot should turn the next time
// it scans for an opponent.
Direction scanDir = DirectionLeft;

// This gets set to true whenever we change to a new state.
// A state can read and write this variable this in order to
// perform actions just once at the beginning of the state.
bool justChangedState;

// This gets set whenever we clear the display.
bool displayCleared;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  // Uncomment if necessary to correct motor directions:
  //motors.flipLeftMotor(true);
  //motors.flipRightMotor(true);

  lineSensors.initThreeSensors();
  proxSensors.initThreeSensors();

  changeState(StateInit);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop()
{
  bool buttonPress = buttonA.getSingleDebouncedPress();

  if (state == StateInit) //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
  {
    // Stato iniziale della macchina, dove vengono fatte tutte le calibrazioni e poi si aspetta 5 secondi
    motors.setSpeeds(0, 0);

    if (justChangedState)
    {
      justChangedState = false;
      lcd.print(F("Press A"));
    }

    if (displayIsStale(100))
    {
      displayUpdated();
      lcd.gotoXY(0, 1);
      lcd.print(readBatteryMillivolts());
    }

    if (buttonPress)
    {
      // The user pressed button A, so go to the seek state.
      changeState(StateWaiting);
    }

  }

  else if (state == StateWaiting) //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
  {
    // In this state, we wait for a while and then move on to the
    // scanning state.

    motors.setSpeeds(0, 0);

    uint16_t time = timeInThisState();

    if (time < waitTime)
    {
      // Display the remaining time we have to wait.
      uint16_t timeLeft = waitTime - time;
      lcd.gotoXY(0, 0);
      lcd.print(timeLeft / 1000 % 10);
      lcd.print('.');
      lcd.print(timeLeft / 100 % 10);
    }
    else
    {
      // We have waited long enough.  VAI ALLO STATO SEEK.
      changeState(StateSeek);
    }
  }

  else if (state == StateSeek)
    /*
        In this state we drive forward while also looking for the opponent using
        the proximity sensors and checking for the white border.
    */
    if (justChangedState)
    {
      justChangedState = false;
      lcd.print(F("seek"));
    }

  // Check for borders.
  lineSensors.read(lineSensorValues);
  if (lineSensorValues[0] < lineSensorThreshold)
  {
    scanDir = DirectionRight;
    changeState(StateBacking);
  }
  if (lineSensorValues[2] < lineSensorThreshold)
  {
    scanDir = DirectionLeft;
    changeState(StateBacking);
  }

  // LEGGE I SENSORI
  proxSensors.read();
  uint8_t sum = proxSensors.countsFrontWithRightLeds() + proxSensors.countsFrontWithLeftLeds();
  int8_t diff = proxSensors.countsFrontWithRightLeds() - proxSensors.countsFrontWithLeftLeds();

  // DEFINISCO LA DIREZIONE
  // We see something with the front sensor but it is not a strong reading.
  if (proxSensors.countsRightWithRightLeds() >= 5)// || diff >= 1)
  {
    // The right-side reading is stronger, so veer to the right.
    //dir(90);
    motors.setSpeeds(veerSpeedHigh, veerSpeedLow);
    //      int destra = proxSensors.countsRightWithRightLeds();
    //      Serial.print(destra);
    sprintf(buffer, "Gira a destra \n");
    Serial.print(buffer);
  }
  else if (proxSensors.countsLeftWithLeftLeds() >= 5)// || diff <= -1)
  {
    // The left-side reading is stronger, so veer to the left.
    //dir(-90);
    //      int sinistra = proxSensors.countsRightWithLeftLeds();
    //      Serial.print(sinistra);
    motors.setSpeeds(veerSpeedLow, veerSpeedHigh);
    sprintf(buffer, "Gira a sinistra \n");
    Serial.print(buffer);
  }
  else if (sum >= 4 || timeInThisState() > stalemateTime)
  {
    // The front sensor is getting a strong signal, or we have been driving forward for a while now without seeing the border.  Either way, there is probably a robot in front of us and we should switch to ramming speed to try to push the robot out of the ring.
    motors.setSpeeds(rammingSpeed, rammingSpeed);
    sprintf(buffer, "Oggetto frontale, vai dritto \n");
    Serial.print(buffer);
    // Turn on the red LED when ramming.
    ledRed(1);
    //      // Both readings are equal, so just drive forward.
    //      motors.setSpeeds(forwardSpeed, forwardSpeed);
    //      sprintf(buffer,"Destra = sinistra, vai dritto \n");
    //      Serial.print(buffer);
  }

  //    if (sum >= 4 || timeInThisState() > stalemateTime) ///////////////////////////////////////////////////////////////////////////////////// Cambio la soglia (prima era 4)
  //    {
  //      // The front sensor is getting a strong signal, or we have been driving forward for a while now without seeing the border.  Either way, there is probably a robot in front of us and we should switch to ramming speed to try to push the robot out of the ring.
  //      motors.setSpeeds(rammingSpeed, rammingSpeed);
  //      sprintf(buffer,"Oggetto frontale, vai dritto \n");
  //      Serial.print(buffer);
  //      // Turn on the red LED when ramming.
  //      ledRed(1);
  //    }

  changeState(StateSeek);

}

}// FINE LOOP

// Gets the amount of time we have been in this state, in
// milliseconds.  After 65535 milliseconds (65 seconds), this
// overflows to 0.
uint16_t timeInThisState()
{
  return (uint16_t)(millis() - stateStartTime);
}

// Changes to a new state.  It also clears the LCD and turns off
// the LEDs so that the things the previous state were doing do
// not affect the feedback the user sees in the new state.
void changeState(uint8_t newState)
{
  state = (State)newState;
  justChangedState = true;
  stateStartTime = millis();
  ledRed(0);
  ledYellow(0);
  ledGreen(0);
  lcd.clear();
  displayCleared = true;
}

// Returns true if the display has been cleared or the contents
// on it have not been updated in a while.  The time limit used
// to decide if the contents are staled is specified in
// milliseconds by the staleTime parameter.
bool displayIsStale(uint16_t staleTime)
{
  return displayCleared || (millis() - displayTime) > staleTime;
}

// Any part of the code that uses displayIsStale to decide when
// to update the LCD should call this function when it updates the
// LCD.
void displayUpdated()
{
  displayTime = millis();
  displayCleared = false;
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
    sprintf(buffer, "-> verso= %" PRIu32 " rotation %4d story_rotation %4d \n", verso, rotation , story_rotation);
    //Serial.print(buffer);
    rotation = rotation + story_rotation; // TODO
    sprintf(buffer, "-> verso= %" PRIu32 " rotation %4d story_rotation %4d \n", verso, rotation , story_rotation);
    //Serial.print(buffer);
    if (rotation == 0)
      rotation = -story_rotation;
    //Serial.print(buffer);
    story_rotation = rotation;
    //    if (rotation > 180) {
    //      rotation = rotation - 360;
    //      sprintf(buffer, "%4d rotation over 180 \n", rotation);
    //      Serial.print(buffer);
    //    }
    //    else if (rotation < -180) {
    //      rotation = rotation + 360;
    //      sprintf(buffer, "%4d rotation over -180 \n", rotation);
    //      Serial.print(buffer);
    //    }
    verso = rotation * turnAngle1;
    sprintf(buffer, "After multiply %" PRIu32 "= %4d * %" PRIu32 " \n", verso, rotation , turnAngle1);
    //Serial.print(buffer);
  }
  dteta = (((int32_t)verso >> 16 ) * 360) >> 16;
  teta = (((int32_t)turnAngle >> 16) * 360) >> 16;
  //printDisplay(0, 0, (int32_t)dteta );
  //printDisplay(3, 0, (int32_t)teta );
  //printDisplay(5, 0, (int32_t)(((int32_t)(story_rotation * turnAngle1) >> 16 ) * 360) >> 16 );
  while (dteta > teta) {
    turnSensorUpdate();
    turnSpeed = (int32_t)verso / (turnAngle1 / 56) - turnRate / 20;
    mot(-turnSpeed, turnSpeed);
    dteta = (((int32_t)verso >> 16 ) * 360) >> 16;
    teta = (((int32_t)turnAngle >> 16) * 360) >> 16;
    //printDisplay(0, 0, (int32_t)dteta );
    //printDisplay(4, 0, (int32_t)teta );
  }
  mot((int32_t)0);
  changeState(StateSeek);
}

void mot(int32_t all) {
  all = constrain(all, -SPEED_MAX, SPEED_MAX);
  motors.setSpeeds(all, all);
  //printDisplay(1, 1, all);
  //printDisplay(4, 1, "|");
  //printDisplay(5, 1, all);
}
void mot(int32_t left, int32_t right) {
  left = constrain(left, -SPEED_MAX, SPEED_MAX);
  right = constrain(right, -SPEED_MAX, SPEED_MAX);
  motors.setSpeeds(left, right);
}

