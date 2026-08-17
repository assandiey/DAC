/*
 * lecture_adc.c
 *
 *  Created on: Dec 17, 2025
 *      Author: assanedieye
 */


#include "lecture_adc.h"

#define ADC_TIMEOUT_MS  10U

static ADC_HandleTypeDef *s_hadc = NULL;
static float s_vref_V = 3.3f;
static uint32_t s_canal = ADC_CHANNEL_0;

static uint32_t lecture_adc_maxCounts(ADC_HandleTypeDef *hadc)
{
    switch (hadc->Init.Resolution)
    {
        case ADC_RESOLUTION_12B: return 4095U;
        case ADC_RESOLUTION_10B: return 1023U;
        case ADC_RESOLUTION_8B:  return 255U;
        case ADC_RESOLUTION_6B:  return 63U;
        default:                 return 4095U;
    }
}

void lecture_adc_configurer(ADC_HandleTypeDef *hadc, float vref_V, uint32_t canal_adc)
{
    s_hadc = hadc;
    s_vref_V = vref_V;
    s_canal = canal_adc;
}

float lire_adc(void)
{
    if (s_hadc == NULL) return -1.0f;

    ADC_ChannelConfTypeDef cfg = {0};
    cfg.Channel = s_canal;
    cfg.Rank = 1;
    cfg.SamplingTime = ADC_SAMPLETIME_144CYCLES;

    if (HAL_ADC_ConfigChannel(s_hadc, &cfg) != HAL_OK)
        return -1.0f;

    if (HAL_ADC_Start(s_hadc) != HAL_OK)
        return -1.0f;

    if (HAL_ADC_PollForConversion(s_hadc, ADC_TIMEOUT_MS) != HAL_OK)
    {
        HAL_ADC_Stop(s_hadc);
        return -1.0f;
    }

    uint32_t brut = HAL_ADC_GetValue(s_hadc);
    HAL_ADC_Stop(s_hadc);

    uint32_t max_counts = lecture_adc_maxCounts(s_hadc);
    return ((float)brut * s_vref_V) / (float)max_counts;
}
