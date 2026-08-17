#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32l152xe.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l1xx_hal_gpio.h"

#include "main.h"

volatile bool flag_temperature_high = false;
volatile bool temperature_checking_high = true;
volatile uint16_t temp_threshold_high = 0;
volatile uint16_t temp_threshold_low = 0;

extern ADC_HandleTypeDef hadc;
extern void SystemClock_Config(void);

uint16_t adc_buffer[2];

float adc_to_celsius(uint16_t raw_adc) {
    uint16_t cal1 = TS_CAL1_ADDR;
    uint16_t cal2 = TS_CAL2_ADDR;

    uint16_t vref_raw = adc_buffer[1];
    float vref = calculate_vref(vref_raw);
    float raw_scaled = (float)raw_adc * (vref / 3.0f);

    float temp_c = ((TS_CAL2_CELSIUS - TS_CAL1_CELSIUS) / (float)(cal2 - cal1))
                   * (raw_scaled - (float)cal1)
                   + TS_CAL1_CELSIUS;

    return temp_c;
}

void print_temp() {
    uint16_t raw_val = adc_buffer[0];
    float current_temp_c = adc_to_celsius(raw_val);
    float fractional = current_temp_c - (uint16_t)current_temp_c;
    uint16_t fracint = fractional * 10;
    printf("Temperature is: %u.%u °C\r\n\r\n", (uint16_t)current_temp_c, fracint);
}

void app_main() {
    HAL_ADC_Start_DMA(&hadc, (uint32_t*)adc_buffer, 2);
    // Wait a bit for the DMA to fill in the VREF value
    HAL_Delay(10);
    // Update the temperature thresholds now
    // since we got the vref from DMA
    temp_threshold_high = calculate_temperature_threshold(TEMP_THRESHOLD_HIGH_C, adc_buffer[1]);
    temp_threshold_low = calculate_temperature_threshold(TEMP_THRESHOLD_LOW_C, adc_buffer[1]);
    hadc.Instance->LTR = 0;
    hadc.Instance->HTR = temp_threshold_high;

    // Explicitly enable the watchdog interrupt bit
    // because we started the ADC with DMA
    // This enables HAL_ADC_LevelOutOfWindowCallback
    __HAL_ADC_ENABLE_IT(&hadc, ADC_IT_AWD);

    while(1) {
        if (flag_temperature_high) {
            // I don't reset the flag here because I want the alarm condition to stick.
            printf("Temperature has reached the threshold of %d °C.\r\n", (uint8_t)TEMP_THRESHOLD_HIGH_C);
            printf("My finger is on the button!\r\n");
            print_temp();
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            HAL_Delay(250);
        } else {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
            printf("Temperature is back to normal. Noch einmal bitte.\r\n");
            printf("My finger is away from the button. Just relax man.\r\n");
            print_temp();
            HAL_Delay(1000);
        }
    }
}

// Window temperature check ISR with hysteresis
void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        if (temperature_checking_high) {
            // We were checking for high temp and got the alarm condition and
            // it got very hot in here!
            // Now we'll check for lower threshold to see if it gets back to normal.
            hadc->Instance->LTR = temp_threshold_low;
            hadc->Instance->HTR = 0xFFFF;
            temperature_checking_high = false;
            flag_temperature_high = true;
        } else {
            // We were checking for normal temp and got back to normal
            // Temps are normal again! YAY!
            // Now we'll check for higher threshold to see if it exceeds that again.
            hadc->Instance->LTR = 0;
            hadc->Instance->HTR = temp_threshold_high;
            temperature_checking_high = true;
            flag_temperature_high = false;
        }
    }
}
