#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "stm32l1xx_hal.h"

#include "main.h"

#define DEBOUNCE_TIME_MS 50

typedef enum {
    SYS_STATE_LOW_POWER,
    SYS_STATE_ACTIVE
} SystemState_t;

volatile SystemState_t current_state = SYS_STATE_LOW_POWER;
volatile uint32_t last_trigger_tick = 0;
volatile uint16_t adc_value = 0;
volatile bool flag_pending_debounce = false;
volatile bool flag_adc_value_ready = false;

extern ADC_HandleTypeDef hadc;
extern COMP_HandleTypeDef hcomp2;
extern TIM_HandleTypeDef htim2;
extern void SystemClock_Config();

void app_main(void) {
    HAL_COMP_Start_IT(&hcomp2);

    if (HAL_COMP_GetOutputLevel(&hcomp2) == COMP_OUTPUTLEVEL_HIGH) {
        current_state = SYS_STATE_ACTIVE;
    }

    while (1) {
        if (flag_pending_debounce && (HAL_GetTick() - last_trigger_tick >= DEBOUNCE_TIME_MS)) {
            flag_pending_debounce = false;

            uint32_t stable_level = HAL_COMP_GetOutputLevel(&hcomp2);

            if (stable_level == COMP_OUTPUTLEVEL_HIGH && current_state == SYS_STATE_LOW_POWER) {
                current_state = SYS_STATE_ACTIVE;
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
                printf("CPU Awakened!\r\n");

                HAL_ADC_Start_IT(&hadc);
                HAL_TIM_Base_Start(&htim2);

            } else if (stable_level == COMP_OUTPUTLEVEL_LOW && current_state == SYS_STATE_ACTIVE) {
                current_state = SYS_STATE_LOW_POWER;
                HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
                printf("Going low power. Hushhh...\r\n");

                HAL_SuspendTick();

                // Enter STOP mode with Regulator in Low Power mode
                // COMP EXTI line 22 remains active to wake up the system
                HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

                // After comparator interrupt fires
                // Resume ticks (ewww) and  restore system clocks upon waking up
                SystemClock_Config(); // Required after stop mode
                HAL_ResumeTick();
            }
        }

        if (flag_adc_value_ready) {
            flag_adc_value_ready = false;
            printf("ADC Value: %u\r\n", adc_value);
        }
    }
}

void HAL_COMP_TriggerCallback(COMP_HandleTypeDef *hcomp) {
    if (hcomp->Instance == COMP2) {
        last_trigger_tick = HAL_GetTick();
        flag_pending_debounce = true;
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    adc_value = HAL_ADC_GetValue(hadc);
    flag_adc_value_ready = true;
}

