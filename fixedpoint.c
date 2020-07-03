#include "./fixedpoint.h"

//Relentlessly stolen from: https://www.eetimes.com/fixed-point-math-in-c-2/#

/**
 * @brief Multiplies two fixed point represented numbers with each other
 * @param a fixed number to be multiplied
 * @param b fixed number to be multiplied
 * @return result of the multiplication
 */
int16_t fp_multiply(int16_t a, int16_t b)
{
    int32_t tmp = (((int32_t)a) * ((int32_t)b) + (1 << (FPART - 1))) >> FPART;
    return ((int16_t)tmp);
}

int32_t fp_multiply32(int32_t a, int32_t b)
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

int16_t fp_cos(int16_t i)
{
    i += 0x0192;
    // printf("i: %04x\t", i);


    i = fp_multiply(0x145f, i);
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
