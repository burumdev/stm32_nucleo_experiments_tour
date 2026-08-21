#ifndef __EEPROM_H
#define __EEPROM_H

// The start address is the same for all STM32L1 series devices
#define EEPROM_BASE_ADDR         0x08080000UL
// For STM32L152RE, the EEPROM size is 16 KB
#define EEPROM_SIZE              16384UL // 16 KB
#define EEPROM_END_ADDR          (EEPROM_BASE_ADDR + EEPROM_SIZE) // 0x08084000

#endif
