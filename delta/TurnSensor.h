#pragma once

#include <Zumo32U4.h>

const int32_t turnAngle45 = 0x20000000;

const int32_t turnAngle90 = turnAngle45 * 2;

const int32_t turnAngle1 = (turnAngle45 + 22) / 45;

void turnSensorSetup();
void turnSensorReset();
void turnSensorUpdate();
extern uint32_t turnAngle;
extern int16_t turnRate;

extern Zumo32U4ButtonA buttonA;
extern Zumo32U4LCD lcd;
extern L3G gyro;
