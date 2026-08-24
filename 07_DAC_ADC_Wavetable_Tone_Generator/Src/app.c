#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "main.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l1xx_hal_dac.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_tim.h"

#include "app.h"
#include "wavetable.h"

#define BUTTON_DEBOUNCE 100
#define MIN_FREQ        20
#define MAX_FREQ        10000
#define UINT12_MAX      4095

extern DAC_HandleTypeDef hdac;
extern ADC_HandleTypeDef hadc;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

volatile bool flag_change_waveform_request = false;
volatile bool flag_change_waveform = false;
volatile uint16_t adc_value = 0;
volatile uint16_t last_adc_value = 0;
volatile bool flag_adc_updated = false;
volatile uint32_t last_button_tick = 0;

app_state_t AppState = {
    .wavetable_index = WTSquare,
};

/*
uint16_t adc_to_frequency_simple(uint16_t adc_value) {
    // Map 0-4095 to 20-20000 Hz audible range
     return MIN_FREQ + ((float)adc_value / UINT12_MAX) * (MAX_FREQ - MIN_FREQ);
}
*/

uint16_t adc_to_frequency_log(uint16_t adc_value) {
    float min_log = log10f((float)MIN_FREQ);
    float max_log = log10f((float)MAX_FREQ);

    // Use 4095.0f for 12-bit ADC resolution
    float normalized = (float)adc_value / (float)UINT12_MAX;
    float log_freq = min_log + normalized * (max_log - min_log);

    return (uint16_t)powf(10.0f, log_freq);
}

uint16_t calculate_timer_period(uint16_t frequency) {
    // With 128 element wave buffers,
    // the effective input frequency
    // for the timer is FREQCLK / 128 = 32000000 / 128 = 250000
    // freq_dac_out = 250000 / (period + 1)
    // period = (250000 / freq_dac_out) - 1
    uint32_t period = (uint32_t)((250000.0f / frequency) - 1.0f);

    if (period > UINT16_MAX) {
        period = UINT16_MAX;
    } else if (period < 1) {
        period = 1;
    }

    return (uint16_t)period;
}

void start_dac() {
    HAL_DAC_Start_DMA(
        &hdac,
        DAC_CHANNEL_1,
        (uint32_t*)WAVETABLES[AppState.wavetable_index],
        WAVE_SIZE,
        DAC_ALIGN_12B_R
    );
}

void app_main() {
    // Start timer 2 to drive the DAC DMA
    HAL_TIM_Base_Start_IT(&htim2);
    start_dac();
    // Start ADC in DMA continuous mode
    HAL_ADC_Start_DMA(&hadc, (uint32_t*)&adc_value, 1);

    while(1) {
        if (flag_adc_updated) {
            flag_adc_updated = false;
            if (adc_value != last_adc_value) {
                uint16_t freq = adc_to_frequency_log(adc_value);
                uint16_t period = calculate_timer_period(freq);
                __HAL_TIM_SET_AUTORELOAD(&htim2, period);
                __HAL_TIM_SET_COUNTER(&htim2, 0);
                last_adc_value = adc_value;
            }
        }
        if (flag_change_waveform) {
            flag_change_waveform = false;
            // Not stopping the DAC here might be possible
            // Using some clever double buffer technique
            HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
            AppState.wavetable_index =
                AppState.wavetable_index == WAVETABLE_TOTAL - 1 ? 0 :
                AppState.wavetable_index + 1;
            start_dac();
        }
    }
}

// Waiting for DAC conversion finish event to change waveform
// the click event is not handled in main directly but delegated after
// DAC conversion is complete to prevent glitch that
// might be introduced to the audio output.
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
    if (flag_change_waveform_request) {
        flag_change_waveform_request = false;
        flag_change_waveform = true;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == B1_Pin) {
        uint32_t current_tick = HAL_GetTick();
        if ((current_tick - last_button_tick) > BUTTON_DEBOUNCE) {
            last_button_tick = current_tick;
            flag_change_waveform_request = true;
        }
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    // Not checking adc peripheral here because the board has one ADC
    flag_adc_updated = true;
}
