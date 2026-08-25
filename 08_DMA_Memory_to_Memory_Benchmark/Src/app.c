#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "stm32l152xe.h"
#include "stm32l1xx_hal.h"
#include "stm32l1xx_hal_dma.h"

// For 80kb of RAM this will result in a sufficiently large copy size.
// 8192 * 4 = 32768 bytes one buffer
// 32768 * 2 = 65536 bytes two buffers
#define BUFFER_SIZE    8192
#define CYCLE_TIME_US  (1.0f / 32000000.0f) * 1000000.0f

extern DMA_HandleTypeDef hdma_memtomem_dma1_channel1;

uint32_t buffer_source[BUFFER_SIZE] = {0};
uint32_t buffer_destination[BUFFER_SIZE] = {0};
uint32_t cycles_memcpy = 0;
uint32_t cycles_dma_m2m_handoff = 0;
volatile uint32_t cycles_start = 0;
volatile uint32_t cycles_dma_m2m_complete = 0;
volatile bool flag_dma_m2m_completed = false;
uint32_t float_tuple[2] = {0};

void dma_transfer_completed_callback(DMA_HandleTypeDef* hdma);

void app_main() {
    for (size_t i = 0; i < BUFFER_SIZE; i++) {
        buffer_source[i] = buffer_source[i] + 42;
    }

    HAL_DMA_RegisterCallback(
        &hdma_memtomem_dma1_channel1,
        HAL_DMA_XFER_CPLT_CB_ID,
        dma_transfer_completed_callback
    );

    // Enable DWT or Data Watchpoint Trigger hardware
    // for elapsed cycles information
    // It has a cycle counter within the Cortex-M core
    // This is a more precise and performant way to benchmark stuff
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable trace unit
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    while(1) {
        // Reset ISR callback flag
        flag_dma_m2m_completed = false;
        // Reset cycle counter
        DWT->CYCCNT = 0;

        // Measure memcpy operation
        cycles_start = DWT->CYCCNT;
        memcpy(buffer_destination, buffer_source, BUFFER_SIZE * sizeof(uint32_t));
        cycles_memcpy = DWT->CYCCNT - cycles_start;

        // Fire up DMA transfer of the same source buffer to destination
        cycles_start = DWT->CYCCNT;
        HAL_DMA_Start_IT(
            &hdma_memtomem_dma1_channel1,
            (uint32_t)buffer_source,
            (uint32_t)buffer_destination,
            BUFFER_SIZE
        );
        cycles_dma_m2m_handoff = DWT->CYCCNT - cycles_start;

        // CPU is free to do other work while DMA is copying in the background
        while (!flag_dma_m2m_completed) {
            // Do something useful here
            // CPU is completely available
        }

        // Publish the results to science magazine, national geographic or the smithsonian
        uint32_t ns = cycles_memcpy * CYCLE_TIME_US;
        printf("Memcpy completed in: \t\t\t%lu cycles, ~%lu microseconds\r\n", cycles_memcpy, ns);
        ns = cycles_dma_m2m_handoff * CYCLE_TIME_US;
        printf("DMA handoff to main happened in: \t%lu cycles, ~%lu microseconds\r\n", cycles_dma_m2m_handoff, ns);
        ns = cycles_dma_m2m_complete * CYCLE_TIME_US;
        printf("DMA transfer completed in: \t\t%lu cycles, ~%lu microseconds\r\n\r\n", cycles_dma_m2m_complete, ns);

        // Wait 3 seconds for the father, the son and holy spirit
        // Or if you prefer for Brahma, Shiva and Vishnu
        HAL_Delay(3000);
    }
}

void dma_transfer_completed_callback(DMA_HandleTypeDef* hdma) {
    cycles_dma_m2m_complete = DWT->CYCCNT - cycles_start;
    flag_dma_m2m_completed = true;
}

