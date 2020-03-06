#include "./compression.h"

/**
 * @brief Transforms data into the DCT domian 
 * @param input_vector The input values given in FP representation.
 * @param result The result vector
 * @param block_size The size of the block to DCT transform
 */
static void dct_transform(int16_t *input_vector_and_result, unsigned int block_size)
{
    int32_t sum, fac, tmp1, iter, iter2, half, imme;
    // int16_t ;
    int16_t result[SIGNAL_LEN];

    // Should be precomputed
    fac = 0x00003243;
    //  / block_size; /* PI / block_size*/
    // fac = float_to_fixed32(3.14159265359) / block_size; /* PI / block_size*/

    // printf("HALF %08x\n\n",float_to_fixed32(3.14159265359));
    // half = 0x00008000;             /* 0.5 */
    half = 0x00080000; /* 0.5 */
    // half = float_to_fixed32(0.5);

    // printf("Fact: %08x\n", fac);
    // printf("Half: %08x\n", half);

    //Set iterators fractions to 0
    for (iter2 = 0; iter2 < block_size; iter2++)
    {
        sum = 0;
        for (iter = 0; iter < block_size; iter++)
        {
            tmp1 = FP.fp_multiply32(FP.fp_multiply32(half + (iter << NPART), (iter2 << NPART)), fac);
            imme = FP.fp_multiply32(((int32_t)input_vector_and_result[iter]) << (NPART - 8), ((int32_t)FP.fp_cos(tmp1 >> (NPART - 8))) << (NPART - 8));
            sum += imme;
            // printf("%d. SUM : %.2f \t", iter, FP.fixed_to_float16(FP.fp_multiply(half + (iter << FPART), (iter2 << FPART))));
            // printf("Imme %d: %.4f \t",iter, fixed_to_float32(imme));
            // printf("SUM %d : %.2f \t", iter,fixed_to_float32(sum));
            // printf("tmp %d : %.2f \t",iter,fixed_to_float32(tmp1));
            // printf("tmmp2 : %.2f \t",fixed_to_float32(fp_multiply32(float_to_fixed32(1.0),float_to_fixed32(1.0))));
            // printf("tmmp3 : %08x \t", float_to_fixed32(1.0));
        }

        // printf("\n\n");
        result[iter2] = (int16_t)(sum >> (NPART - 8));
    }
    for (iter = 0; iter < block_size; iter++)
    {
        input_vector_and_result[iter] = result[iter];
    }
}

static const int16_t c[256] = { 16,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,
 22,22,22,22,22,22,22,22,22,22,22,22,22,22,22,21,21,21,21,21,21,21,21,21,
 21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,20,20,20,20,20,20,20,20,20,
 20,20,20,20,20,20,20,20,19,19,19,19,19,19,19,19,19,19,19,19,19,19,18,18,
 18,18,18,18,18,18,18,18,18,18,18,17,17,17,17,17,17,17,17,17,17,17,16,16,
 16,16,16,16,16,16,16,16,16,15,15,15,15,15,15,15,15,15,14,14,14,14,14,14,
 14,14,14,14,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,12,11,11,11,
 11,11,11,11,11,11,10,10,10,10,10,10,10,10, 9, 9, 9, 9, 9, 9, 9, 9, 8, 8,
  8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 7, 6, 6, 6, 6, 6, 6, 6, 6, 5, 5, 5,
  5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2,
  2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };

static void dct_64_256(int16_t *input_vector_and_result, unsigned int block_size)
{
    int16_t result[SIGNAL_LEN] = {0};
    int16_t m = 0, n = 0;
    int8_t sign = 1;
    int32_t sum = 0;
    for (m = 0; m < 64; m++)
    {
        sum = 0;
        for (n = 0; n < SIGNAL_LEN; n++)
        {
            sign = INDEX_FORMULA(m,n) / SIGNAL_LEN / 2 % 2 == 0 ? -1 : 1;
            if ((INDEX_FORMULA(m,n) / SIGNAL_LEN) % 2 == 0)
            {
                sum += FP.fp_multiply32((int32_t)(input_vector_and_result[n]) << (NPART - FPART), -sign * (int32_t)(c[INDEX_FORMULA(m,n) % SIGNAL_LEN]) << (NPART - FPART));
            }
            else
            {
                sum += FP.fp_multiply32((int32_t)(input_vector_and_result[n]) << (NPART - FPART), sign * ((int32_t)(c[SIGNAL_LEN - INDEX_FORMULA(m,n) % SIGNAL_LEN]) << (NPART - FPART)));
                // LOG_INFO_("%02x * %02x = %02x\n", input_vector_and_result[n], c[n*(m*2+1)], (uint16_t)FP.fp_multiply32((int32_t)(input_vector_and_result[n] << (NPART - 8)), (int32_t)(c[n*(m*2+1)]<< (NPART - 8)) >> (NPART - 8)));
            }
        }
        result[m] = (int16_t)(sum >> (NPART - FPART));
    }
    memset(input_vector_and_result, 0, BLOCK_LEN);
    memcpy(input_vector_and_result, result, 64);
}

/**
 * @brief Thresholds an array of to a given threshold
 * @param dct_vector The input DCT vector
 * @param threshold The given threshold
 * @param length The length of the DCT vector 
 */
static void threshold(int16_t *dct_vector, int16_t threshold, uint16_t length)
{
    uint16_t i;

    for (i = 0; i < length; i++)
    {
        if (dct_vector[i] < 0) /* Check if negative*/
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

// void mem_copy(uint8_t *dest, uint8_t *src, uint16_t length)
// {
//     uint16_t i;
//     for (i = 0; i < length; i++)
//     {
//         dest[i] = src[i];
//     }
// }

int pushBits(huffman_codeword huff_code, uint8_t *bitstring, uint16_t bitstring_length)
{
    uint8_t i = huff_code.word_length;
    uint8_t last_byte_bit = bitstring_length % 8;
    while (i--)
    {
        bitstring[bitstring_length++ >> 3] |= (((huff_code.word >> i) & 1U) << (7 - last_byte_bit));
        last_byte_bit = bitstring_length % 8;
    }
    return bitstring_length;

    // Not so optimized but more readable code - does the same
    // uint8_t i = huff_code.word_length;
    // uint8_t last_byte_bit = bitstring_length % 8;
    // uint16_t bitstr_len = bitstring_length;
    // uint8_t bit = 0x0;
    // while (i--) {
    // bit = ((huff_code.word >> i) & 1U);
    // bitstring[bitstr_len >> 3] |= (bit << (7 - last_byte_bit));
    // bitstr_len++;
    // last_byte_bit = bitstr_len % 8;
    // }
    // return bitstr_len;
}

static huffman_metadata huffman_encode(uint8_t *block_and_result, uint16_t length, const huffman_codeword *codebook, const huffman_codeword h_eof)
{
    huffman_metadata h_data;
    uint16_t i;
    uint8_t firstHalf;
    uint8_t secondHalf;
    uint8_t bitstring[BLOCK_LEN * 2];
    uint16_t bitstr_len = 0;
    huffman_codeword huff_code1, huff_code2;
    uint8_t remainder;
    memset(bitstring, 0, BLOCK_LEN * 2);
    for (i = 0; i < length; i++)
    {
        // Push first half to final huff code
        firstHalf = (block_and_result[i] & 0xF0) >> 4;
        huff_code1 = codebook[firstHalf];
        secondHalf = block_and_result[i] & 0x0F;
        huff_code2 = codebook[secondHalf];
        // // If the huff code exceeds original length - there is no point in doing it
        // // Return success = -1 and keep the block as is
        if (((bitstr_len >> 3) + huff_code1.word_length + huff_code2.word_length + h_eof.word_length) > (BLOCK_LEN - 1))
        {
            h_data.byte_length = BLOCK_LEN;
            h_data.success = -1;
            return h_data;
        }
        bitstr_len = pushBits(huff_code1, bitstring, bitstr_len);
        bitstr_len = pushBits(huff_code2, bitstring, bitstr_len);
    }
    // When done push eof
    bitstr_len = pushBits(h_eof, bitstring, bitstr_len);

    remainder = bitstr_len % 8 ? 1 : 0;

    h_data.byte_length = (bitstr_len >> 3) + remainder;
    memset(block_and_result, 0, length);
    memcpy(block_and_result, bitstring, h_data.byte_length);
    h_data.length = bitstr_len;
    h_data.success = 1;

    return h_data;
}
const struct compression_driver compression_driver = {
    dct_transform,
    threshold,
    // simple_truncate,
    huffman_encode,
    dct_64_256};
