#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "main.h"
#include "eeprom.h"
#include "eepromdata.h"
#include "stm32l1xx_hal_crc.h"
#include "stm32l1xx_hal_def.h"
#include "stm32l1xx_hal_flash_ex.h"

extern CRC_HandleTypeDef hcrc;

void app_main() {
    printf("Calculating CRC from file data...\r\n");
    uint32_t fileDataCRC = HAL_CRC_Calculate(&hcrc, EEPROM_DATA, EEPROM_DATA_SIZE);
    printf("File data CRC calculated as: %lx\r\n\r\n", fileDataCRC);

    printf("Writing data to EEPROM...\r\n");
    HAL_FLASHEx_DATAEEPROM_Unlock();
    HAL_StatusTypeDef writeStatus = HAL_OK;
    for (size_t i = 0; i < EEPROM_DATA_SIZE; i++) {
        size_t eeprom_address = EEPROM_BASE_ADDR + (i * 4);
        writeStatus = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, eeprom_address, EEPROM_DATA[i]);
        if (writeStatus != HAL_OK) {
            printf("An error occured while writing data to eeprom!\r\n");
            break;
        }
    }
    HAL_FLASHEx_DATAEEPROM_Lock();

    if (writeStatus == HAL_OK) {
        printf("Reading data from EEPROM...\r\n");
        uint32_t readbuf[EEPROM_DATA_SIZE] = {0};
        for (size_t i = 0; i < EEPROM_DATA_SIZE; i++) {
            readbuf[i] = *((uint32_t *)(EEPROM_BASE_ADDR + (i * 4)));
        }
        printf("Calculating CRC from EEPROM data...\r\n");
        uint32_t eepromCRC = HAL_CRC_Calculate(&hcrc, readbuf, EEPROM_DATA_SIZE);
        printf("EEPROM data CRC calculated as: %lx\r\n\r\n", eepromCRC);
        if (eepromCRC == fileDataCRC) {
            printf("File CRC and EEPROM CRC match. Data is valid. Congratulations!\r\n");
        }
    } else {
        printf("Can't continue without successful data write. Looping endlessly now...\r\n");
        while(1);
    }

    while(1) {}
}
