/*
 * communication_uart.h
 *
 *  Created on: Dec 17, 2025
 *      Author: assanedieye
 */

#ifndef INC_COMMUNICATION_UART_H_
#define INC_COMMUNICATION_UART_H_


#include "stm32f4xx_hal.h"
#include <stdint.h>

#include "stm32f4xx_hal.h"

void communication_uart_configurer(UART_HandleTypeDef *huart);

HAL_StatusTypeDef envoyerValeur(float tension_V);
HAL_StatusTypeDef envoyerTexte(const char *texte);

#endif /* INC_COMMUNICATION_UART_H_ */
