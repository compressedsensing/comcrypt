#include "./fixedpoint.h"

//Relentlessly stolen from: https://www.eetimes.com/fixed-point-math-in-c-2/#

#if FLOAT
static double fixed_to_float16(int16_t input)
{
    double res = 0;
    res = ((double)input / (double)(1 << FPART));
    return res;
}

static int16_t float_to_fixed16(double input)
{
    int16_t res;
    res = (int16_t)(input * (1 << FPART));
    return res;
}

static double fixed_to_float32(int32_t input)
{
    double res = 0;
    res = ((double)input / (double)(1 << NPART));
    return res;
}

static int32_t float_to_fixed32(double input)
{
    int32_t res;
    res = (int32_t)(input * (1 << NPART));
    return res;
}
#endif

/**
 * @brief Multiplies two fixed point represented numbers with each other
 * @param a fixed number to be multiplied
 * @param b fixed number to be multiplied
 * @return result of the multiplication
 */
static int16_t fp_multiply(int16_t a, int16_t b)
{
    // int32_t tmp = 0;
    // // Save result in double size
    // tmp = ((int32_t)a) * ((int32_t)b);
    // // Take out midder section of bits
    // tmp = (tmp + (1 << (FPART - 1))) >> FPART;
    // return ((int16_t)tmp);
    int32_t tmp = (((int32_t)a) * ((int32_t)b) + (1 << (FPART - 1))) >> FPART;
    return ((int16_t)tmp);
    // return (int16_t)(((((int32_t)a) * ((int32_t)b)) + (1 << (FPART - 1))) >> FPART);
}

static int16_t fp_mult(int16_t a, int16_t b)
{
    int32_t tmp;
    int32_t IL;

    // long tmp, Z;
    int16_t result;

    // Save result in double size
    tmp = (int32_t)a * (int32_t)b;

    // Take out midder section of bits
    tmp = tmp + (1 << (FPART - 1));
    tmp = tmp >> FPART;

    // if(tmp > INT16_MAX){
    //     printf("MULTIPLICATION OVERFLOW!!!!");
    // }
    // #ifdef DEBUG
    // #endif

    // // Saturate the result if over or under minimum value.
    // if (tmp > INT32_MAX) /* saturate the result before assignment */
    //     Z = INT32_MAX;
    // else if (tmp < INT32_MIN)
    //     Z = INT32_MIN;
    // else
    
    // if(tmp > INT16_MAX){
    //     printf("MULTIPLICATION OVERFLOW!!!!");
    // }
    if(tmp > INT16_MAX){
        IL = (int32_t)(tmp % 0x00010000);
    }
    // else {
    IL = tmp;
    // }

    result = IL;

    return result;
}

static int32_t fp_multiply32(int32_t a, int32_t b)
{
    int64_t tmp;
    int64_t IL;

    int32_t result;

    // Save result in double size
    tmp = (int64_t)a * (int64_t)b;

    // Take out midder section of bits
    tmp += 0x00080000;
    

    tmp = tmp >> NPART;

    // Saturate the result if over or under minimum value.

    IL = tmp;

    result = IL;

    return result;
}

static int32_t fp_16_to_32(int16_t a) {
    int32_t result = a;
    return result << (NPART - FPART);
}

static int16_t fp_32_to_16(int32_t a) {
    int16_t result = a >> (NPART - FPART);
    return result;
}

static int16_t fp_cos(int16_t i)
{
    i += 0x0192;
    // printf("i: %04x\t", i);


    i = fp_mult(0x145f, i);
    /* Convert (signed) input to a value between 0 and 8192. (8192 is pi/2, which is the region of the curve fit). */
    /* ------------------------------------------------------------------- */
    i <<= 1;
    uint8_t c = i < 0; //set carry for output pos/neg

    // printf("C: %d\n",c);

    if (i == (i | 0x4000)) // flip input value to corresponding value in range [0..8192)
        i = (1 << 15) - i;
    i = (i & 0x7FFF) >> 1;
    /* ------------------------------------------------------------------- */

    /* The following section implements the formula:
     = y * 2^-n * ( A1 - 2^(q-p)* y * 2^-n * y * 2^-n * [B1 - 2^-r * y * 2^-n * C1 * y]) * 2^(a-q)
    Where the constants are defined as follows:
    */
    enum
    {
        A1 = 3370945099UL,
        B1 = 2746362156UL,
        C1 = 292421UL
    };
    enum
    {
        n = 13,
        p = 32,
        q = 31,
        r = 3,
        a = FPART
    };

    uint32_t y = (C1 * ((uint32_t)i)) >> n;
    y = B1 - (((uint32_t)i * y) >> r);
    y = (uint32_t)i * (y >> n);
    y = (uint32_t)i * (y >> n);
    y = A1 - (y >> (p - q));
    y = (uint32_t)i * (y >> n);
    y = (y + (1UL << (q - a - 1))) >> (q - a); // Rounding

    // return y;
    return c ? -y : y;
}

// /**
//  * @brief Taylor approximation of sin function
//  * @param a number inside sin(a)
//  * @param b Order of taylor polynomiel
//  * @return sin(a)
//  */
// static FIXED11_21 fp_sin(FIXED11_21 a, int precision)
// {
//     FIXED11_21 result, minus, div1;
//     int32_t div, sign;
//     result.full = 0; /* Initialize result value*/

//     //Set minus to -1
//     minus.part.fraction = 0;
//     minus.part.integer = -1;

//     sign = 1; /* Set sign used for values in pi:2pi */

//     // Fix input between 0:2*pi
//     a.full = a.full % (2 * FP_PI21_16);

//     if (a.full > FP_PI21_16)
//     {
//         a.full -= FP_PI21_16;
//         sign = -1;
//     }

//     //Taylor expansion
//     int n;
//     for (n = 0; n < precision; n++)
//     {
//         div = factorial((2 * n) + 1);
//         div1.full = fp_multiply(fp_pow(minus, n), fp_pow(a, (2 * n) + 1)).full / div;
//         result = fp_add(result, div1);
//     }

//     result.full = result.full * sign;
//     return result;
// }

// /**
//  * @brief Taylor approximation of cos function
//  * @param a number inside cos(a)
//  * @param b Order of taylor polynomiel
//  * @return cos(a)
//  */
// static FIXED11_21 fp_cos(FIXED11_21 a, int precision)
// {
//     FIXED11_21 div, PI;
//     PI.full = FP_PI21_16;
//     div.full = PI.full / 2;

//     return fp_sin(fp_add(a, div), precision);
// }

#if FLOAT
const struct fixed_point_driver fixed_point_driver = {fp_multiply, fp_multiply32, fp_cos, fixed_to_float16, float_to_fixed16,fixed_to_float32, float_to_fixed32};
#else
const struct fixed_point_driver fixed_point_driver = {fp_multiply, fp_multiply32, fp_cos, fp_16_to_32, fp_32_to_16};
#endif