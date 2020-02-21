#include "./compression.h"

/**
 * @brief Transforms data into the DCT domian 
 * @param input_vector The input values given in FP representation.
 * @param result The result vector
 * @param block_size The size of the block to DCT transform
 */
static void dct_transform(int16_t *input_vector_and_result, unsigned int block_size)
{
    int16_t fac, half, iter, iter2, sum;
    int16_t result[SIGNAL_LEN];

    // Should be precomputed
    fac = 0x0324 / block_size; /* PI / block_size*/
    half = 0x0080; /* 0.5 */

    //Set iterators fractions to 0
    for (iter2 = 0; iter2 < block_size; iter2++)
    {
        sum = 0;
        for (iter= 0; iter < block_size; iter++)
        {
            sum += FP.fp_multiply(input_vector_and_result[iter],FP.fp_cos(FP.fp_multiply(FP.fp_multiply(half + (iter << FPART), (iter2 << FPART)), fac)));
            // printf("SUM : %.2f \t",FP.fixed_to_float16(sum));
        }

        // printf("\n\n");
        result[iter2] = sum;
    }
    for (iter = 0; iter < SIGNAL_LEN; iter++)
    {
        input_vector_and_result[iter] = result[iter];
    }
}

/**
 * @brief Thresholds an array of to a given threshold
 * @param dct_vector The input DCT vector
 * @param threshold The given threshold
 * @param length The length of the DCT vector 
 */
static void threshold(int16_t *dct_vector, int16_t threshold, unsigned int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        if ((dct_vector[i] << FPART) < 0)
        {
            if (-dct_vector[i] < threshold)
            {
                dct_vector[i] = 0;
            }
        }
        else
        {
            if (dct_vector[i] < threshold)
            {
                dct_vector[i] = 0;
            }
        }
    }
}

// static void simple_truncate(FIXED11_21 *dct_vector, FIXED11_21 *result, uint16_t length, uint16_t result_length)
// {
//     uint16_t i;
//     for (i = 0; i < result_length; i++)
//     {
//         result[i].full = dct_vector[i].full;
//     }
// }

int pushBits(huffman_codeword huff_code, uint8_t *bitstring, uint16_t bitstring_length)
{
    uint16_t max_code_word_size = sizeof(uint16_t) * 8;
    uint16_t sig_bit = 1 << (max_code_word_size - 1);
    uint8_t i;
    uint8_t last_byte_bit = bitstring_length % 8;
    uint16_t bitstr_len = bitstring_length;
    uint16_t start = max_code_word_size - huff_code.word_length;
    uint8_t bit;
    for (i = 0; i < huff_code.word_length; i++) {
        bit = (((huff_code.word << (start + i)) & sig_bit) >> last_byte_bit) >> 8;
        bitstring[bitstr_len >> 3] |= bit;
        bitstr_len++;
        last_byte_bit = bitstr_len % 8;
    }
    return bitstr_len;
}

static huffman_metadata huffman_encode(uint8_t *block_and_result, uint16_t length, const huffman_codeword *codebook, const huffman_codeword h_eof)
{
    huffman_metadata h_data;
    uint16_t i;
    uint8_t firstHalf;
    uint8_t secondHalf;
    uint8_t bitstring[BLOCK_LEN * 2] = {0};
    uint16_t bitstr_len = 0;
    huffman_codeword huff_code;
    uint8_t remainder;
    uint16_t byte_length;
    for (i = 0; i < length; i++)
    {
        // Push first half to final huff code
        firstHalf = (block_and_result[i] & 0xF0) >> 4;
        huff_code = codebook[firstHalf];
        bitstr_len = pushBits(huff_code, bitstring, bitstr_len);

        // Repeat for second half
        secondHalf = block_and_result[i] & 0x0F;
        huff_code = codebook[secondHalf];
        bitstr_len = pushBits(huff_code, bitstring, bitstr_len);
    }
    // When done push eof
    bitstr_len = pushBits(h_eof, bitstring, bitstr_len);

    remainder = bitstr_len % 8 ? 1 : 0;

    byte_length = (bitstr_len >> 3) + remainder;
    h_data.byte_length = byte_length;
    memset(block_and_result, 0, length);
    memcpy(block_and_result, bitstring, byte_length);
    h_data.length = bitstr_len;
    h_data.success = byte_length > length ? -1 : 1;

    return h_data;
}
const struct compression_driver compression_driver = {
    dct_transform, 
    threshold, 
    // simple_truncate, 
    huffman_encode
};
