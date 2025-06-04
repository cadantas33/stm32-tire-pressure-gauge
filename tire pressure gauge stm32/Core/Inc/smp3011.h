/*
 * smp3011.h
 * Created on: May 26, 2025
 * Author: clvol
 * 1. Vermelho: Vcc
 * 2. Preto: Gnd
 * 3. Amarelo: SDA
 * 4. Azul: SCL
 */

#ifndef INC_SMP3011_H_
#define INC_SMP3011_H_

#include "main.h"

void smp3011_init(I2C_HandleTypeDef *hi2c);
uint32_t smp3011_get_temperature();
uint32_t smp3011_get_pressure();
void smp3011_read();

#endif /* INC_SMP3011_H_ */
