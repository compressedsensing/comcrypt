#include "./compression.h"

/**
 * @brief Transforms data into the DCT domian 
 * @param input_vector The input values given in FP representation.
 * @param result The result vector
 * @param block_size The size of the block to DCT transform
 */
static void dct_transform(FIXED11_21 *input_vector, FIXED11_21 *result, unsigned int block_size)
{

    FIXED11_21 fac, half, iter, iter2, sum;
    fac.full = FP_PI21_16 / block_size;

    FIXED11_21 ;
    half.full = 0b00000000000000000000010000000000;
    
    //Set iterators fractions to 0
    iter.part.fraction = 0;
    iter2.part.fraction = 0;

    for (iter2.part.integer = 0; iter2.part.integer < block_size; iter2.part.integer++)
    {   
        sum.full = 0;
        for (iter.part.integer = 0; iter.part.integer < block_size; iter.part.integer++)
        {
            sum = FP.fp_add(sum, FP.fp_multiply(input_vector[iter.part.integer], FP.fp_cos(FP.fp_multiply(FP.fp_multiply(FP.fp_add(half, iter), iter2), fac), 5)));
        }
        result[iter2.part.integer] = sum;
    }
}

/**
 * @brief Thresholds an array of to a given threshold
 * @param dct_vector The input DCT vector
 * @param threshold The given threshold
 * @param length The length of the DCT vector 
 */
static void threshold(float *dct_vector, float threshold, unsigned int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        if (dct_vector[i] < threshold)
        {
            dct_vector[i] = 0.0;
        }
    }
}

const struct compression_driver compression_driver = {dct_transform, threshold};