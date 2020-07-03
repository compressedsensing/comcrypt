#ifndef FP_H_
#define FP_H_

#include "contiki.h"
#include <inttypes.h>
#include <stdio.h>
#include "./configuration.h"

// #define FP_PI 0b00000000011001001000011111101101
// #define FP_PI21_16 0b00000000000000000001100100100001

#define IPART 4
#define FPART 12
#define NPART 20

int16_t fp_multiply(int16_t a, int16_t b);
int32_t fp_multiply32(int32_t a, int32_t b);
int16_t fp_cos(int16_t a);

#endif /* FP_H_ */