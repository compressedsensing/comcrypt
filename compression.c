#include "./compression.h"

//TODO add comment
static void dct_transform(float *input_vector, float *result, unsigned int block_size)
{
    float factor = M_PI / block_size;
    int i, j; //Predefine loop reference

    for (i = 0; i < block_size; i++)
    {
        float sum = 0;
        for (j = 0; j < block_size; j++)
            sum += input_vector[j] * cos((j + 0.5) * i * factor);
        result[i] = sum;
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