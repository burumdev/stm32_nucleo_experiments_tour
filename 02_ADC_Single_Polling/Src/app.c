#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l1xx_hal_def.h"

#include <stdint.h>
#include <stdio.h>

extern ADC_HandleTypeDef hadc;

void app_main() {
    while(1) {
        HAL_Delay(500);
        HAL_ADC_Start(&hadc);

        if (HAL_ADC_PollForConversion(&hadc, 100) == HAL_OK) {
            uint32_t adcVal = HAL_ADC_GetValue(&hadc);
            uint32_t voltage_mv = (adcVal * 3300) / 4096;
            printf("ADC:\tValue: %lu\tVoltage: %lu millivolts\r\n", adcVal, voltage_mv);
        } else {
            printf("ERROR: ADC read timeout...\r\n");
        }
    }
}
