#ifndef FP_H_
#define FP_H_

#include "contiki.h"
#include <inttypes.h>
#include <stdio.h>
#include "./configuration.h"

#define FP fixed_point_driver
// #define FP_PI 0b00000000011001001000011111101101
// #define FP_PI21_16 0b00000000000000000001100100100001

#define IPART 8
#define FPART 8
#define NPART 20

struct fixed_point_driver
{
    int16_t (*fp_multiply)(int16_t a, int16_t b);
    int32_t (*fp_multiply32)(int32_t a, int32_t b);
    int16_t (*fp_cos)(int16_t a);
    int32_t (*fp_16_to_32)(int16_t a);
    int16_t (*fp_32_to_16)(int32_t a);
#if FLOAT

    double (*fixed_to_float16)(int16_t input);
    int16_t (*float_to_fixed16)(double input);
    double (*fixed_to_float32)(int32_t input);
    int32_t (*float_to_fixed32)(double input);
#endif
};

extern const struct fixed_point_driver FP;

#endif /* FP_H_ */