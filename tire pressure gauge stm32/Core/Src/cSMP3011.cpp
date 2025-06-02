#include "cSMP3011.h"


#define SMP3011_MAX_RANGE_PA            (500000.0f)
#define SMP3011_MIN_RANGE_PA            (0.0f)

#define SMP3011_PRESSURE_RESOLUTION     (16777216.0f)
#define SMP3011_TEMPERATURE_RESOLUTION  (65536.0f)
#define SMP3011_ADDRESS                 (0x78 << 1) // Endere�o I2C no formato HAL (7-bit address shifted left)
#define SMP3011_MAX_TEMPERATURE         (150.0f)
#define SMP3011_MIN_TEMPERATURE         (-40.0f)

#define SMP3011_MAX_PRESSURE_PERCENTAGE (0.85f)
#define SMP3011_MIN_PRESSURE_PERCENTAGE (0.15f)

#define SMP3011_START_CONVERSION        0xAC

cSMP3011::cSMP3011()
{
    temperature = 0;
    pressure = 0;
    hi2c = nullptr;
}

cSMP3011::~cSMP3011()
{

}

void cSMP3011::init(I2C_HandleTypeDef *hi2c)
{
    this->hi2c = hi2c;
    
    if(this->hi2c != nullptr)
    {
        uint8_t PressSensorCommand = SMP3011_START_CONVERSION;
        HAL_I2C_Master_Transmit(this->hi2c, SMP3011_ADDRESS, &PressSensorCommand, 1, HAL_MAX_DELAY);
    }
}

float cSMP3011::getTemperature()
{    
    return temperature;
}

float cSMP3011::getPressure()
{
    return pressure;
}

HAL_StatusTypeDef cSMP3011::poll()
{
    if(hi2c == nullptr)
    {
        return HAL_ERROR;
    }

    uint8_t PressSensorBuffer[6];
    HAL_StatusTypeDef status;
    
    // L� os dados do sensor
    status = HAL_I2C_Master_Receive(hi2c, SMP3011_ADDRESS, PressSensorBuffer, sizeof(PressSensorBuffer), HAL_MAX_DELAY);
    if(status != HAL_OK)
    {
        return status;
    }
    
    // Verifica se os dados est�o prontos 
    if((PressSensorBuffer[0] & 0x20) == 0)
    {
        // Inicia nova convers�o
        uint8_t PressSensorCommand = SMP3011_START_CONVERSION;
        status = HAL_I2C_Master_Transmit(hi2c, SMP3011_ADDRESS, &PressSensorCommand, 1, HAL_MAX_DELAY);
        if(status != HAL_OK)
        {
            return status;
        }

        // Processa os dados de press�o
        float pressurePercentage = (((uint32_t)PressSensorBuffer[1] << 16) | 
                                  ((uint32_t)PressSensorBuffer[2] << 8) | 
                                  ((uint32_t)PressSensorBuffer[3]));
        pressurePercentage /= SMP3011_PRESSURE_RESOLUTION;

        // Processa os dados de temperatura
        float temperaturePercentage = ((uint32_t)PressSensorBuffer[4] << 8) | 
                                     ((uint32_t)PressSensorBuffer[5]);
        temperaturePercentage /= SMP3011_TEMPERATURE_RESOLUTION;
        
        // Converte para valores reais
        pressure = (((pressurePercentage - SMP3011_MIN_PRESSURE_PERCENTAGE) / 
                   (SMP3011_MAX_PRESSURE_PERCENTAGE - SMP3011_MIN_PRESSURE_PERCENTAGE)) * 
                   (SMP3011_MAX_RANGE_PA - SMP3011_MIN_RANGE_PA)) + SMP3011_MIN_RANGE_PA;
        
        temperature = ((SMP3011_MAX_TEMPERATURE - SMP3011_MIN_TEMPERATURE) * 
                      temperaturePercentage) + SMP3011_MIN_TEMPERATURE;
    }
    
    return HAL_OK;
}
