/*
 * conversion_tension.c
 *
 *  Created on: Dec 17, 2025
 *      Author: assanedieye
 */

float convertirVadcVersVin(float vadc_V)
{
    const float Vcc = 3.3f;
    const float R1  = 16000.0f;
    const float R2  = 10000.0f;
    const float R3  = 33000.0f;
    const float R4  = 10000.0f;

    const float Vref = Vcc * (R2 / (R1 + R2));
    const float k    = R4 / R3;

    return (Vref * (1.0f + k) - vadc_V) / k;
}
