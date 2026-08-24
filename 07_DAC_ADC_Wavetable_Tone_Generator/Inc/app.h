#ifndef __APP_H
#define __APP_H

#include <stddef.h>
#include "wavetable.h"

void app_main(void);

typedef struct {
  WaveTable wavetable_index;
} app_state_t;

#endif
