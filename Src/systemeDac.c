/*
 * systemeDac.c
 *
 *  Created on: Dec 17, 2025
 *      Author: assanedieye
 */

#include "systemeDac.h"
#include "main.h"
#include "lecture_adc.h"
#include "conversion_tension.h"
#include "communication_uart.h"

#define PERIODE_MS 100U

void systemeDac(void)
{
    static uint32_t precedent_ms = 0;
    uint32_t maintenant_ms = HAL_GetTick();

    if ((maintenant_ms - precedent_ms) < PERIODE_MS)
        return;

    precedent_ms = maintenant_ms;

    // 1 - Lire ADC
    float tension_adc = lire_adc();

    if (tension_adc < 0.0f)
        return;

    // 2 -  Convertir vers Vin
    float tension_convertie = convertirVadcVersVin(tension_adc);

    // 3 - Envoyer UART
    (void)envoyerValeur(tension_convertie);
}

