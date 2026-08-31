#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "main.h"
#include "app.h"
#include "stm32l152xe.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_adc.h"
#include "stm32l1xx_hal_flash.h"
#include "stm32l1xx_hal_flash_ex.h"
#include "stm32l1xx_hal_gpio.h"
#include "stm32l1xx_hal_tim.h"
#include "TM1638.h"
#include "screen.h"

static volatile uint16_t adc_buffer[ADC_BUFFER_SIZE] = {0};
static volatile size_t adc_index = 0;
static volatile uint16_t adc_value = 0;

static volatile bool flag_adc_record_start = false;
static volatile bool flag_adc_record_playback = false;
static volatile bool flag_adc_conversion = false;
static volatile bool flag_adc_record_save = false;
static volatile bool flag_adc_record_load = false;
static volatile bool flag_buffer_full = false;
static volatile bool is_adc_recording = false;

extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc;

void save_word_to_flash(uint32_t word, size_t position) {
    uint32_t address = FLASH_BANK2_BASE + position;
    HAL_FLASH_Program(FLASH_TYPEPROGRAMDATA_WORD, address, word);
}

void load_from_flash() {
    size_t adc_idx = 0;
    for (uint32_t address = FLASH_BANK2_BASE; address < FLASH_BANK2_BASE + ADC_BUFFER_SIZE_BYTES; address += 4) {
        uint32_t word = *(__IO uint32_t *)address;
        uint16_t first = (uint16_t) (word & 0xFFFFUL);
        adc_buffer[adc_idx] = first;
        uint16_t second = (uint16_t) ((word >> 16) & 0xFFFFUL);
        adc_buffer[adc_idx + 1] = second;
        adc_idx += 2;
    }
}

void erase_flash_data() {
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PAGEError = 0;

    HAL_FLASH_Unlock();

    // Configure erase structure
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    // Start address of Bank 2
    EraseInitStruct.PageAddress = FLASH_BANK2_BASE;
    // 1 page = 256 bytes on this hardware
    EraseInitStruct.NbPages = ADC_BUFFER_SIZE_BYTES / 256;

    // 3. Perform Page Erase
    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK) {
        // Trap the CPU
        while(1);
    }

    HAL_FLASH_Lock();
}

void app_main() {
    TM1638 display;

    display.stb_port = GPIOA;
    display.clk_port = GPIOA;
    display.dio_port = GPIOA;
    display.stb_pin = GPIO_PIN_7;
    display.clk_pin = GPIO_PIN_6;
    display.dio_pin = GPIO_PIN_5;

    tm1638_init(&display, 5);
    tm1638_display_clear(&display);

    while(1) {
        if (flag_adc_conversion && is_adc_recording) {
            flag_adc_conversion = false;
            print_number(&display, adc_value, 1);
            update_progress_leds(&display, adc_index);
        }
        if (flag_adc_record_start) {
            flag_adc_record_start = false;
            is_adc_recording = true;

            adc_index = 0;
            adc_value = 0;
            memset((void*)adc_buffer, 0, ADC_BUFFER_SIZE * sizeof(uint16_t));

            tm1638_display_clear(&display);
            // Tim2 update event will trigger ADC conversions
            // Frequency is 50 hertz (20ms period)
            HAL_TIM_Base_Start(&htim2);
            // Start ADC in dma interrupt mode
            HAL_ADC_Start_IT(&hadc);
        }
        if (flag_buffer_full || flag_adc_record_playback) {
            flag_buffer_full = false;
            flag_adc_record_playback = false;
            is_adc_recording = false;

            HAL_TIM_Base_Stop(&htim2);
            HAL_ADC_Stop_IT(&hadc);

            for (size_t i = 0; i < ADC_BUFFER_SIZE; i++) {
                uint16_t value = adc_buffer[i];
                print_number(&display, value, 5);
                update_progress_leds(&display, i);

                // Playback can be interrupted and
                // restarted
                if (flag_adc_record_playback) {
                    break;
                } else {
                    // To replicate the envelope accurately
                    // wait for 20ms period (50 hertz frequency)
                    HAL_Delay(20);
                }
            }

            adc_index = 0;
            adc_value = 0;
        }
        if (flag_adc_record_save) {
            flag_adc_record_save = false;

            erase_flash_data();

            HAL_FLASH_Unlock();
            uint32_t word = 0;
            for (size_t i = 0; i < ADC_BUFFER_SIZE; i++) {
                uint16_t value = adc_buffer[i];
                if (i % 2 == 0) {
                    word = value;
                } else {
                    word |= ((uint32_t)value << 16);
                    save_word_to_flash(word, ((i - 1) * 2));
                    word = 0;
                }
            }
            HAL_FLASH_Lock();
        }
        if (flag_adc_record_load) {
            flag_adc_record_load = false;
            load_from_flash();
        }
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC1 && is_adc_recording) {
        if (adc_index == ADC_BUFFER_SIZE) {
            flag_buffer_full = true;
        } else {
            adc_value = HAL_ADC_GetValue(hadc);
            adc_buffer[adc_index] = adc_value;
            adc_index++;
            flag_adc_conversion = true;
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch(GPIO_Pin) {
        case RecordStart_Pin:
            flag_adc_record_start = true;
            break;
        case RecordPlayback_Pin:
            flag_adc_record_playback = true;
            break;
        case RecordSave_Pin:
            flag_adc_record_save = true;
            break;
        case RecordLoad_Pin:
            flag_adc_record_load = true;
            break;
        default:
            break;
    }
}
