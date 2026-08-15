#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l1xx_hal_def.h"

#include "app.h"
#include "main.h"
#include "stm32l1xx_hal_gpio.h"

extern ADC_HandleTypeDef hadc;

volatile bool changeADCResolutionFlag = false;

void app_main() {
    const uint32_t ADC_RESOLUTIONS[4] = {
        ADC_RESOLUTION_12B,
        ADC_RESOLUTION_10B,
        ADC_RESOLUTION_8B,
        ADC_RESOLUTION_6B
    };
    const char* ADC_RESOLUTION_NAMES[4] = {
        "12 bit",
        "10 bit",
        "8 bit",
        "6 bit",
    };
    const uint16_t ADC_RESOLUTION_DIVISORS[4] = {
        4096,
        1024,
        256,
        64,
    };

    size_t adc_resolution_index = 0;

    while(1) {
        if (changeADCResolutionFlag) {
            changeADCResolutionFlag = false;

            HAL_ADC_Stop(&hadc);
            adc_resolution_index = adc_resolution_index == 3 ? 0 : adc_resolution_index + 1;
            uint32_t resolution = ADC_RESOLUTIONS[adc_resolution_index];
            hadc.Init.Resolution = resolution;
            printf("Changed ADC resolution to: %s\r\n", ADC_RESOLUTION_NAMES[adc_resolution_index]);
            HAL_ADC_Init(&hadc);
        }

        HAL_Delay(500);

        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        HAL_ADC_Start(&hadc);

        if (HAL_ADC_PollForConversion(&hadc, 100) == HAL_OK) {
            uint32_t adcVal = HAL_ADC_GetValue(&hadc);
            uint32_t voltage_mv = (adcVal * 3300) / ADC_RESOLUTION_DIVISORS[adc_resolution_index];
            printf("ADC:\tValue: %lu\tVoltage: %lu millivolts\r\n", adcVal, voltage_mv);
        } else {
            printf("ERROR: ADC read timeout...\r\n");
        }
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == B1_Pin) {
        changeADCResolutionFlag = true;
    }
}
