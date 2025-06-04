/*
 * smp3011.c
 * Created on: May 26, 2025
 * Author: clvol
 */

#include "smp3011.h"

#define SMP3011_MAX_RANGE_PA            (500000.0f)
#define SMP3011_MIN_RANGE_PA            (0.0f)

#define SMP3011_PRESSURE_RESOLUTION     (16777216.0f)
#define SMP3011_TEMPERATURE_RESOLUTION  (65536.0f)
#define SMP3011_ADDRESS                 (0x78 << 1)
#define SMP3011_MAX_TEMPERATURE         (150.0f)
#define SMP3011_MIN_TEMPERATURE         (-40.0f)

#define SMP3011_MAX_PRESSURE_PERCENTAGE (0.85f)
#define SMP3011_MIN_PRESSURE_PERCENTAGE (0.15f)

#define SMP3011_START_CONVERTION        0xAC

I2C_HandleTypeDef *_smp3011_ui2c;
uint32_t _smp3011_temp;
uint32_t _smp3011_pres;

void smp3011_init(I2C_HandleTypeDef *hi2c)
{
  _smp3011_ui2c = hi2c;
}

uint32_t smp3011_get_temperature()
{
  return _smp3011_temp;
}

uint32_t smp3011_get_pressure()
{
  return _smp3011_pres;
}

void smp3011_read()
{
  if(_smp3011_ui2c)
  {
    uint8_t cmd = SMP3011_START_CONVERTION;
    //ptI2C->masterWrite(SMP3011_ADDRESS, &PressSensorCommand, 1);
    uint8_t arr[1] = { cmd };
    HAL_I2C_Master_Transmit(_smp3011_ui2c, SMP3011_ADDRESS, arr, sizeof(arr), HAL_MAX_DELAY);
    HAL_Delay(310);

    uint8_t PressSensorBuffer[6];
    //ptI2C->masterRead(SMP3011_ADDRESS, PressSensorBuffer, sizeof(PressSensorBuffer));
    HAL_I2C_Master_Receive(_smp3011_ui2c, SMP3011_ADDRESS, PressSensorBuffer, sizeof(PressSensorBuffer), HAL_MAX_DELAY);

    if((PressSensorBuffer[0]&0x20) == 0)
    {
      float pressurePercentage = (((uint32_t)PressSensorBuffer[1]<<16)|((uint32_t)PressSensorBuffer[2]<<8)|((uint32_t)PressSensorBuffer[3]));
      pressurePercentage /= SMP3011_PRESSURE_RESOLUTION;

      float temperaturePercentage = ((uint32_t)PressSensorBuffer[4]<<8)|((uint32_t)PressSensorBuffer[5]);
      temperaturePercentage /= SMP3011_TEMPERATURE_RESOLUTION;

      float _pres = (((pressurePercentage - SMP3011_MIN_PRESSURE_PERCENTAGE)/(SMP3011_MAX_PRESSURE_PERCENTAGE - SMP3011_MIN_PRESSURE_PERCENTAGE)) * (SMP3011_MAX_RANGE_PA-SMP3011_MIN_RANGE_PA)) + SMP3011_MIN_RANGE_PA;
      float _temp = ((SMP3011_MAX_TEMPERATURE - SMP3011_MIN_TEMPERATURE)*temperaturePercentage) + SMP3011_MIN_TEMPERATURE;
      _smp3011_pres = (uint32_t) _pres;
      _smp3011_temp = (uint32_t) _temp;

    }
  }
}

