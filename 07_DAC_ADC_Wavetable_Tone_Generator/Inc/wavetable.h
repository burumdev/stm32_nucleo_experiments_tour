#ifndef WAVETABLE_H
#define WAVETABLE_H

#include <stdint.h>

#define WAVE_SIZE 128
#define WAVETABLE_TOTAL 4

extern uint16_t* WAVETABLES[WAVETABLE_TOTAL];

typedef enum {
  WTSquare,
  WTSaw,
  WTSine,
  WTTriangle,
} WaveTable;

#endif
