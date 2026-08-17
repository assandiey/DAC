/*
 * lecture_adc.h
 *
 *  Created on: Dec 17, 2025
 *      Author: assanedieye
 */

#ifndef INC_LECTURE_ADC_H_
#define INC_LECTURE_ADC_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

void lecture_adc_configurer(ADC_HandleTypeDef *hadc, float vref_V, uint32_t canal_adc);

float lire_adc(void);

#endif /* INC_LECTURE_ADC_H_ */
