#include "util.h"

void uint8_to_str(uint8_t num, char *str) {
    if (num == 0) {
        *str++ = '0';
        *str = '\0';
        return;
    }

    char buf[4];

    uint8_t i = 0;
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i > 0) {
        *str++ = buf[--i];
    }

    *str = '\0';
}

void uint16_to_str(uint16_t num, char *str) {
  if (num == 0) {
    *str++ = '0';
    *str = '\0';
    return;
  }

  char buf[6];

  uint8_t i = 0;
  while (num > 0) {
    buf[i++] = (num % 10) + '0';
    num /= 10;
  }

  while (i > 0) {
    *str++ = buf[--i];
  }

  *str = '\0';
}

void uint32_to_str(uint32_t num, char *str) {
  if (num == 0) {
    *str++ = '0';
    *str = '\0';
    return;
  }

  char buf[11];

  uint8_t i = 0;
  while (num > 0) {
    buf[i++] = (num % 10) + '0';
    num /= 10;
  }

  while (i > 0) {
    *str++ = buf[--i];
  }

  *str = '\0';
}
