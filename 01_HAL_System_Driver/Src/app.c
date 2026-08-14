#include <stdio.h>
#include <stdbool.h>

#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_gpio.h"

#include "main.h"
#include "app.h"

volatile bool toggleTickFlag = false;

void app_main() {
    bool tickSuspended = false;
    printf("Hal version: %lu\r\n", HAL_GetHalVersion());
    printf("Device: %lu\r\n", HAL_GetDEVID());
    printf("Device revision: %lu\r\n", HAL_GetREVID());

    while(1) {
        if (toggleTickFlag) {
            toggleTickFlag = false;

            if (tickSuspended) {
                tickSuspended = false;
                printf("Resuming count...\r\n");
                HAL_ResumeTick();
            } else {
                tickSuspended = true;
                printf("Suspending count...\r\n");
                HAL_SuspendTick();
            }
        }

        if (!tickSuspended) {
            HAL_Delay(1000);
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
            printf("Tick count: %lu\r\n", HAL_GetTick());
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == B1_Pin) {
        toggleTickFlag = true;
    }
}
