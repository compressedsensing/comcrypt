#include "./fixedpoint.h"

//Relentlessly stolen from: https://www.eetimes.com/fixed-point-math-in-c-2/#

/**
 * Helper function that prints a 32-bit fractional bits 
 */
static void printBits32(int32_t num)
{
    for (int bit = 0; bit < (sizeof(int32_t) * 8); bit++)
    {
        printf("%i ", num & 0x01);
        num = num >> 1;
    }
}

/**
 * Helper function that prints a 64-bit fractional bits 
 */
static void printBits64(int64_t num)
{
    for (int bit = 0; bit < (sizeof(int64_t) * 8); bit++)
    {
        printf("%li ", num & 0x01);
        num = num >> 1;
    }
}

/**
 * @brief Multiplies two fixed point represented numbers with each other
 * @param a fixed number to be multiplied
 * @param b fixed number to be multiplied
 * @return result of the multiplication
 */
static FIXED11_21 fp_multiply(FIXED11_21 a, FIXED11_21 b)
{
    int64_t tmp, Z;
    FIXED11_21 result;

    //Save result in double size
    tmp = (int64_t)a.full * (int64_t)b.full;

    //Take out midder section of bits
    tmp = tmp + (1 << 20);
    tmp = tmp >> 21;

    //Saturate the result if over or under minimum value.
    if (tmp > INT32_MAX) /* saturate the result before assignment */
        Z = INT32_MAX;
    else if (tmp < INT32_MIN)
        Z = INT32_MIN;
    else
        Z = tmp;

    result.full = Z;

    return result;
}

/**
 * @brief Multiplies two fixed point represented numbers with each other
 * @param a fixed number to be multiplied
 * @param b fixed number to be multiplied
 * @return result of the multiplication
 */
static FIXED11_21 fp_add(FIXED11_21 a, FIXED11_21 b)
{
    FIXED11_21 result;

    result.full = a.full + b.full; /* Has a risk of overflowing */

    return result;
}

/**
 * @brief Multiplies two fixed point represented numbers with each other
 * @param a fixed number to be multiplied
 * @param b fixed number to be multiplied
 * @return result of the multiplication
 */
static FIXED11_21 fp_subtract(FIXED11_21 a, FIXED11_21 b)
{
    FIXED11_21 result;

    result.full = a.full - b.full; /* Has a risk of overflowing */

    return result;
}


/**
 * @brief Converts a fixed point representation number to a double
 * @param input The fixed point value
 * @return float(double) representation of input value
 */
static double fixed_to_float(FIXED11_21 input)
{
    double res = 0;
    res = ((double)input.full / (double)(1 << 21));
    return res;
}

/**
 * @brief Coverts a float(double) to a fixed_point double 
 * @param input The input float
 * @return  fixed point reprensentation of a float
 */
static FIXED11_21 float_to_fixed(double input)
{
    FIXED11_21 res;
    res.full = (int32_t)(input * (1 << 21));
    return res;
}

static FIXED11_21 fp_pow(FIXED11_21 a, int b)
{
    int i;
    FIXED11_21 result = a;

    if (b == 0)
    {
        result.part.integer = 1;
        result.part.fraction = 0;
        return result;
    }

    for (i = 0; i < b - 1; i++)
    {
        // printf("Prev: %f\n", fixed_to_float(result));
        // printf("Initial %f\n", fixed_to_float(a));
        result = fp_multiply(result, a);
        // printf("Result%f\n\n", fixed_to_float(result));
    }

    // printf("Final Result%f\n\n", fixed_to_float(result));
    return result;
}

static int32_t factorial(int a)
{
    int32_t result = 1;
    int i;

    for (i = 1; i < a; i++)
    {
        result += result * i;
    }
    return result;
}

static FIXED11_21 fp_division(FIXED11_21 a, FIXED11_21 b)
{

    int64_t tmp = 0;
    FIXED11_21 result;

    tmp = (int64_t)a.full << 21;
    tmp = tmp + (b.full >> 1);
    tmp = tmp / b.full;

    result.full = (uint32_t)tmp;
    return result;
}

static FIXED11_21 fp_sin(FIXED11_21 a, int precision)
{
    FIXED11_21 result, minus, div1;
    int32_t div;
    result.full = 0;
    // minus.part.fraction = 0;
    minus = float_to_fixed(-1);
    // printf("Real %f\n", fixed_to_float(minus));

    fp_pow(minus, 1);

    int n;

    for (n = 0; n < precision; n++)
    {
        div = factorial((2*n)+1);
        // div.part.integer = factorial((2 * n) + 1);
        // div.part.fraction = 0;

        printf("Switcher: %f\n",fixed_to_float(fp_pow(minus, n)));
        printf("Numerator: %f\n",fixed_to_float(fp_multiply(fp_pow(minus, n), fp_pow(a, (2 * n) + 1))));
        // printf("DIV: %f\n",fixed_to_float(div));
        div1.full = fp_multiply(fp_pow(minus, n), fp_pow(a, (2 * n) + 1)).full / div;
        result = fp_add(result, div1);

        // result = fp_add(result, (fp_division((fp_multiply(fp_pow(minus, n), fp_pow(a, (2 * n) + 1))), div)));
    }
    return result;
}

static FIXED11_21 fp_cos(FIXED11_21 a, int precision)
{

    FIXED11_21 div, PI;
    PI.full = FP_PI;
    div.full = PI.full / 2;
    
    return fp_sin(fp_add(a,div), precision);
}

const struct fixed_point_driver fixed_point_driver = {fp_multiply, fp_division, fp_add, fp_subtract, fp_pow, fp_sin, fp_cos, factorial, fixed_to_float, float_to_fixed};