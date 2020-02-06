#ifndef FP_H_
#define FP_H_

#include "contiki.h"
#include <inttypes.h>
#include <stdio.h>

#define FP fixed_point_driver
#define FP_PI 0b00000000011001001000011111101101
#define FP_PI21_16 0b00000000000000000001100100100001

#define IPART 21
#define FPART 11

typedef union FIXED11_21tag {
    int32_t full;
    struct part11_21tag
    {
        int32_t fraction : FPART;
        int32_t integer : IPART;
    } part;
} FIXED11_21;

struct fixed_point_driver
{
    FIXED11_21(* fp_multiply)(FIXED11_21 a, FIXED11_21 b);
    FIXED11_21(* fp_division)(FIXED11_21 a, FIXED11_21 b);
    FIXED11_21(* fp_add)(FIXED11_21 a, FIXED11_21 b);
    FIXED11_21(* fp_subtract)(FIXED11_21 a, FIXED11_21 b);
    FIXED11_21(* fp_pow)(FIXED11_21 a, int b);
    FIXED11_21(* fp_sin)(FIXED11_21 a, int precision);
    FIXED11_21(* fp_cos)(FIXED11_21 a, int precision);
    int32_t(* factorial)(int a);
    /* Helpers */
    double (* fixed_to_float)(FIXED11_21 input);
    FIXED11_21 (* float_to_fixed)(double input);
};

extern const struct fixed_point_driver FP;

#endif /* FP_H_ */