/*
 * communication_uart.c
 *
 *  Created on: Dec 17, 2025
 *      Author: assanedieye
 */

#include "communication_uart.h"
#include <string.h>
#include <stdio.h>

#define COMM_UART_TIMEOUT_MS 100U

static UART_HandleTypeDef *s_huart = NULL;

void communication_uart_configurer(UART_HandleTypeDef *huart)
{
    s_huart = huart;
}

HAL_StatusTypeDef envoyerTexte(const char *texte)
{
    if (s_huart == NULL || texte == NULL) return HAL_ERROR;
    return HAL_UART_Transmit(s_huart, (uint8_t*)texte, (uint16_t)strlen(texte), COMM_UART_TIMEOUT_MS);
}

HAL_StatusTypeDef envoyerValeur(float tension_V)
{
    if (s_huart == NULL) return HAL_ERROR;

    int32_t tension_mV = (int32_t)(tension_V * 1000.0f + (tension_V >= 0 ? 0.5f : -0.5f));

    char msg[20];
    int n = snprintf(msg, sizeof(msg), "%ld\r\n", (long)tension_mV);
    if (n <= 0) return HAL_ERROR;

    return HAL_UART_Transmit(s_huart, (uint8_t*)msg, (uint16_t)n, COMM_UART_TIMEOUT_MS);
}
