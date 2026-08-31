#ifndef SCREEN_H
#define SCREEN_H

#include "TM1638.h"

void update_progress_leds(TM1638* display, size_t index);
void print_number(TM1638* display, uint16_t number, size_t offset);

#endif
