#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

void uint8_to_str(uint8_t num, char *str);
void uint16_to_str(uint16_t num, char *str);
void uint32_to_str(uint32_t num, char *str);

#endif

