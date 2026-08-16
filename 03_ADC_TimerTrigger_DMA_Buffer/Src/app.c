#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "stm32l152xe.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_uart.h"

#include "main.h"
#include "util.h"

#define APP_ADC_BUF_LEN     128
#define APP_ADC_BUF_HALF    ((APP_ADC_BUF_LEN) / 2)

extern TIM_HandleTypeDef htim3;
extern ADC_HandleTypeDef hadc;
extern UART_HandleTypeDef huart2;

volatile bool flag_adc_first_half_complete = false;
volatile bool flag_adc_second_half_complete = false;

// Buffer to hold 12 bit ADC values
static uint16_t adc_buffer[APP_ADC_BUF_LEN] = {0};

void print_adc_buffer_half(size_t start_idx) {
    for (size_t i = start_idx; i < start_idx + APP_ADC_BUF_HALF; i++) {
        uint16_t value = adc_buffer[i];
        uint16_t voltage = ((uint32_t)value * 3300UL) / 4096UL;

        // Bump sizes by 2-3 bytes to prevent any overflow
        char samplebuf[8] = {0};
        char valuebuf[8] = {0};
        char voltbuf[8] = {0};

        uint8_to_str((uint8_t)i + 1, samplebuf);
        uint16_to_str(value, valuebuf);
        uint16_to_str(voltage, voltbuf);

        HAL_UART_Transmit(&huart2, (uint8_t*)"\t", 1, 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)samplebuf, strlen(samplebuf), 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)"\t", 1, 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)valuebuf, strlen(valuebuf), 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)"\t", 1, 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)voltbuf, strlen(voltbuf), 100);
        HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);
    }
}

void app_main() {
    char* heading = "*** ADC Timer Triggered DMA Buffer ***\r\n";
    HAL_UART_Transmit(&huart2, (uint8_t*)heading, strlen(heading), 100);
    // Start the timer3 for ADC sample triggering
    HAL_TIM_Base_Start(&htim3);
    // Start ADC channel 0 conversions in DMA mode
    HAL_ADC_Start_DMA(&hadc, (uint32_t*)adc_buffer, APP_ADC_BUF_LEN);

    while(1) {
        if (flag_adc_first_half_complete) {
            flag_adc_first_half_complete = false;
            const char *header = "\tSAMPLE\tVALUE\tVOLTAGE\r\n";
            HAL_UART_Transmit(&huart2, (uint8_t*)header, strlen(header), 1000);
            print_adc_buffer_half(0);
        }

        if (flag_adc_second_half_complete) {
            flag_adc_second_half_complete = false;
            print_adc_buffer_half(APP_ADC_BUF_HALF);
        }
    }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
        flag_adc_first_half_complete = true;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1) {
        HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
        flag_adc_second_half_complete = true;
    }
}
