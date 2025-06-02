#pragma once
#ifndef CSMP3011_H
#define CSMP3011_H

#include "stm32c0xx_hal.h"

class cSMP3011
{
private:
    float temperature;
    float pressure;
    I2C_HandleTypeDef *hi2c;

public:
    cSMP3011();
    ~cSMP3011();
    void init(I2C_HandleTypeDef *hi2c);
    float getTemperature();
    float getPressure();
    HAL_StatusTypeDef poll();
};

#endif
