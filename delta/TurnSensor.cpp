#include <Wire.h>
#include "TurnSensor.h"
uint32_t turnAngle = 0;

int16_t turnRate;

int16_t gyroOffset;

uint16_t gyroLastUpdate = 0;

void turnSensorSetup()
{
  Wire.begin();
  gyro.init();

  gyro.writeReg(L3G::CTRL1, 0b11111111);

  gyro.writeReg(L3G::CTRL4, 0b00100000);

  gyro.writeReg(L3G::CTRL5, 0b00000000);

  lcd.clear();
  lcd.print(F("Gyro cal"));

  ledYellow(1);

  delay(1000);

  int32_t total = 0;
  for (uint16_t i = 0; i < 1024; i++)
  {
    while(!gyro.readReg(L3G::STATUS_REG) & 0x08);
    gyro.read();

    total += gyro.g.z;
  }
  ledYellow(0);
  gyroOffset = total / 1024;

  lcd.clear();
}

void turnSensorReset()
{
  gyroLastUpdate = micros();
  turnAngle = 0;
}

void turnSensorUpdate()
{
  gyro.read();
  turnRate = gyro.g.z - gyroOffset;

  uint16_t m = micros();
  uint16_t dt = m - gyroLastUpdate;
  gyroLastUpdate = m;

  int32_t d = (int32_t)turnRate * dt;

  turnAngle += (int64_t)d * 14680064 / 17578125;
}
