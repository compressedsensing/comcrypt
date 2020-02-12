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

    FIXED11_21;
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
static void threshold(FIXED11_21 *dct_vector, FIXED11_21 threshold, unsigned int length)
{
    int i;

    for (i = 0; i < length; i++)
    {
        if (dct_vector[i].part.integer < 0)
        {
            if (-dct_vector[i].full < threshold.full)
            {
                printf("hehre\n");
                dct_vector[i].full = 0;
            }
        }
        else
        {
            printf("COOL\n");
            if (dct_vector[i].full < threshold.full)
            {
                dct_vector[i].full = 0;
            }
        }
    }
}

FIXED11_21 *simple_truncate(FIXED11_21 *dct_vector, FIXED11_21 *result, unsigned int length)
{
    int len = 10;

    if (length < len)
    {
        return;
    }
    FIXED11_21 res[len];

    int i;
    for (i = 0; i < len; i++)
    {
        res[i].full = dct_vector[i].full;
    }

    return res;
}
const uint16_t default_huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
    0x0A98,
    0x0152,
    0x015E,
    0x00A8,
    0x0055,
    0x0028,
    0x000B,
    0x0000,
    0x0003,
    0x0004,
    0x0029,
    0x0056,
    0x00AE,
    0x015F,
    0x02A7,
    0x054D,
};

const uint16_t default_huffman_eof = 0xA99;

uint8_t *calloc_byte(uint16_t nmem)
{
    uint16_t i;
    uint8_t *mem = (uint8_t *)malloc(nmem);
    for (i = 0; i < nmem; i++)
    {
        mem[i] = 0x0;
    }

    return mem;
}

void mem_copy(uint8_t *dest, uint8_t *src, uint16_t length)
{
    uint16_t i;
    for (i = 0; i < length; i++)
    {
        dest[i] = src[i];
    }
}

int pushBits(uint16_t huff_code, uint8_t *bitstring, uint16_t bitstring_length)
{
    uint16_t max_code_word_size = sizeof(uint16_t) * 8;
    uint16_t sig_bit = 1 << (max_code_word_size - 1);
    uint8_t i, j;
    uint8_t last_byte_bit = bitstring_length % 8;
    uint16_t bitstr_len = bitstring_length;
    uint16_t start;
    uint8_t bit;
    for (i = 0; i < max_code_word_size; i++)
    {
        start = (sig_bit >> i) & huff_code;
        if (start)
        {
            // we have arrived at the index of the starting bit
            // Append it to bitstring at bitstring_length
            for (j = 0; j < max_code_word_size - i; i++)
            {
                bit = (((huff_code << (i + j)) & sig_bit) >> last_byte_bit) >> 8;
                bitstring[bitstr_len >> 3] |= bit;
                bitstr_len++;
                last_byte_bit = bitstr_len % 8;
            }
            return bitstr_len;
        }
    }
    // If none return 1 - there should be a single zero which there already is
    return bitstr_len + 1;
}

static struct huffman_data huffman_encode(uint8_t *block, uint16_t length, const uint16_t *codebook, const uint16_t h_eof)
{
    struct huffman_data h_data;
    uint16_t i;
    uint8_t firstHalf;
    uint8_t secondHalf;
    uint8_t bitstring[HUFFMAN_BLOCK_MAX_SIZE * 2] = {0};
    uint16_t bitstr_len = 0;
    uint16_t huff_code;
    uint8_t remainder = 0;
    // bitstring = calloc_byte(length * 2); // Potentially N = 512 Bytes
    for (i = 0; i < length; i++)
    {
        // Push first half to final huff code
        firstHalf = (block[i] & 0xF0) >> 4;
        huff_code = codebook[firstHalf];
        bitstr_len = pushBits(huff_code, bitstring, bitstr_len);

        // Repeat for second half
        secondHalf = block[i] & 0x0F;
        huff_code = codebook[secondHalf];
        bitstr_len = pushBits(huff_code, bitstring, bitstr_len);
    }
    // When done push eof
    bitstr_len = pushBits(h_eof, bitstring, bitstr_len);

    if (bitstr_len % 8)
    {
        remainder = 1;
    }

    h_data.bits = calloc_byte((bitstr_len >> 3) + remainder);
    mem_copy(h_data.bits, bitstring, (bitstr_len >> 3) + remainder);
    h_data.length = bitstr_len;

    if ((h_data.length >> 3) > length)
    {
        h_data.success = -1;
    }
    else
    {
        h_data.success = 1;
    }
    return h_data;
}
const struct compression_driver compression_driver = {dct_transform, threshold, simple_truncate, huffman_encode};
