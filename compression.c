#include "./compression.h"

static void dct_transform(float* input_vector,float* result, unsigned int block_size)
{
    float factor = M_PI / block_size;
    size_t i, j; //Predefine loop reference

    for (i = 0; i < block_size; i++)
    {
        float sum = 0;
        for (j = 0; j < block_size; j++)
            sum += input_vector[j] * cos((j + 0.5) * i * factor);
        result[i] = sum;
    }
}

const struct compression_driver compression_driver = {
  dct_transform
};