#include "./compression.h"

static void dct_transform(float *input_vector, float *result, unsigned int block_size)
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

int pushBits(uint16_t huff_code, uint8_t *bitstring, uint16_t bitstring_length) {

}

static struct huffman_data huffman_encode(uint8_t *block, uint16_t length) {
    uint16_t i;
    uint8_t firstHalf;
    uint8_t secondHalf;
    uint8_t *bitstring;
    uint16_t last_bit_position = 0;
    uint16_t huff_code;
    bitstring = calloc(length, sizeof(uint8_t)); // Potentially N = 512 Bytes
    for (i = 0; i < length; i++) {
        // Push first half to final huff code
        firstHalf = block[i] & 0x0F;
        huff_code = CODEBOOK[firstHalf];
        last_bit_position = pushBits(huff_code, bitstring, last_bit_position);

        // Repeat for second half
        secondHalf = block[i] & 0xF0;
        huff_code = CODEBOOK[secondHalf];
        last_bit_position = pushBits(huff_code, bitstring, last_bit_position);
    }

    struct huffman_data h = {
        last_bit_position,
        bitstring
    };

    return h;
} 

const struct compression_driver compression_driver = {dct_transform};