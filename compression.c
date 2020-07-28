#include "./compression.h"

/*Convolution and Decimation */
static void convolution(const int16_t *signal,
                        size_t signal_length,
                        const int16_t *kernel,
                        size_t kernel_length,
                        int16_t *result)
{
    size_t n;
    for (n = 1; n < signal_length + kernel_length - 1; n += 2) {
        size_t kmin, kmax, k;
        result[n / 2] = 0;
        kmin = (n >= kernel_length - 1) ? n - (kernel_length - 1) : 0;
        kmax = (n < signal_length - 1) ? n : signal_length - 1;

        for (k = kmin; k <= kmax; k++) {
            result[n / 2] += fp_multiply(signal[k], kernel[n - k]);
        }
    }
}

/**
 * @brief Transforms data into the DWT domian 
 * @param input_vector The input values given in FP representation.
 * @param result The result vector
 * @param block_size The size of the block to DCT transform
*/
#define sig_len SIGNAL_LEN //128 //256 
#define k_len 12
#define DWT_FILTER_SIZE 12
#define DWT_RESULT_SIZE sig_len+31 //159 //287 //543
#define BIN_MAP_LEN CEIL_DIVIDE(DWT_RESULT_SIZE, 8)
/* rbio 5.5*/
static const int16_t bio_filter_h[DWT_FILTER_SIZE] = {0, 55, -11, -559, -382, 1952, 3684, 1952, -382, -559, -11, 55};
static const int16_t bio_filter_g[DWT_FILTER_SIZE] = {0, 162, -32, -223, -1415, 3017, -1415, -223, -32, 162, 0, 0};
uint16_t dwt_transform(int16_t *input_vector_and_result)
{
    int16_t res[DWT_RESULT_SIZE / 2] = {0};
    int16_t output[DWT_RESULT_SIZE] = {0};
    uint8_t binary_map[BIN_MAP_LEN] = {0};
    uint16_t output_length = 0;

    int i, ii, j;
    ii = 0;

    convolution(input_vector_and_result, sig_len, bio_filter_g, k_len, res);

    for (i = 0; i < (k_len + sig_len - 1) / 2; i++) {
        output[ii] = res[i];
        ii++;
    }

    convolution(input_vector_and_result, sig_len, bio_filter_h, k_len, res);

    //Decimate
    j = 0;
    for (i = 0; i < (k_len + sig_len - 1) / 2; i++) {
        input_vector_and_result[j] = res[i];
        j++;
    }

    convolution(input_vector_and_result, (k_len + sig_len - 1) / 2 + 11, bio_filter_g, k_len, res);

    for (i = 0; i < ((k_len + sig_len - 1) / 2 + 11) / 2; i++) {
        output[ii] = res[i];
        ii++;
    }

    convolution(input_vector_and_result, (k_len + sig_len - 1) / 2 + 11, bio_filter_h, k_len, res);

    j = 0;
    for (i = 0; i < ((k_len + sig_len - 1) / 2 + 11) / 2; i++) {
        input_vector_and_result[j] = res[i];
        j++;
    }

    convolution(input_vector_and_result, ((k_len + sig_len - 1) / 2 + 11) / 2 + 11, bio_filter_g, k_len, res);

    for (i = 0; i < (((k_len + sig_len - 1) / 2 + 11) / 2 + 11) / 2; i++) {
        output[ii] = res[i];
        ii++;
    }

    convolution(input_vector_and_result, ((k_len + sig_len - 1) / 2 + 11) / 2 + 11, bio_filter_h, k_len, res);

    for (i = 0; i < (((k_len + sig_len - 1) / 2 + 11) / 2 + 11) / 2; i++) {
        output[ii] = res[i];
        ii++;
    }

    /* THRESHOLD AND BINARY MAP */
    int16_t threshold = 25;

    for (i = 0; i < DWT_RESULT_SIZE; i++) {
        uint8_t bin = 1;

        if (output[i] < 0) {
            if (-output[i] < threshold) {
                output[i] = 0;
                bin = 0;
            }
        }
        else {
            if (output[i] < threshold) {
                output[i] = 0;
                bin = 0;
            }
        }

        /* Append binary index correctly */
        binary_map[i / 8] |= (bin << (8 - (i % 8) - 1));

        /* Add signal to output */
        if (bin) {
            output[output_length] = output[i];
            output_length++;
        }
    }

    /* Assign binary map to output*/
    for (i = 0; i < BIN_MAP_LEN / 2; i++) {
        input_vector_and_result[i] = (binary_map[(2 * i)] << 8) | binary_map[(2 * i) + 1];
    }

    for (i = 0; i < output_length; i++) {
        input_vector_and_result[i + (BIN_MAP_LEN / 2)] = output[i];
    }

    return output_length + (BIN_MAP_LEN / 2);
}

/**
 * @brief Transforms data into the DCT domian 
 * @param input_vector The input values given in FP representation.
 * @param result The result vector
 * @param block_size The size of the block to DCT transform
 */
void dct_transform(int16_t *input_vector_and_result, unsigned int block_size)
{
    int32_t sum, fac, tmp1, iter, iter2, half, imme;
    int16_t result[SIGNAL_LEN];

    // Should be precomputed
    fac = 0x00003243;
    half = 0x00080000; /* 0.5 */

    for (iter2 = 0; iter2 < block_size; iter2++)
    {
        sum = 0;
        for (iter = 0; iter < block_size; iter++)
        {
            tmp1 = fp_multiply32(fp_multiply32(half + (iter << NPART), (iter2 << NPART)), fac);
            imme = fp_multiply32(((int32_t)input_vector_and_result[iter]) << (NPART - 8), ((int32_t)fp_cos(tmp1 >> (NPART - 8))) << (NPART - 8));
            sum += imme;
        }
        result[iter2] = (int16_t)(sum >> (NPART - 8));
    }
    for (iter = 0; iter < block_size; iter++)
    {
        input_vector_and_result[iter] = result[iter];
    }
}

static const int16_t c[SIGNAL_LEN] = { 256,362,362,361,361,361,361,361,361,361,361,361,361,360,360,360,360,360,
 359,359,359,359,358,358,358,357,357,357,356,356,355,355,355,354,354,353,
 353,352,352,351,351,350,350,349,348,348,347,347,346,345,345,344,343,343,
 342,341,340,340,339,338,337,336,336,335,334,333,332,331,330,330,329,328,
 327,326,325,324,323,322,321,320,319,318,317,316,315,313,312,311,310,309,
 308,307,305,304,303,302,301,299,298,297,295,294,293,292,290,289,288,286,
 285,284,282,281,279,278,277,275,274,272,271,269,268,266,265,263,262,260,
 259,257,256,254,252,251,249,248,246,244,243,241,239,238,236,234,233,231,
 229,227,226,224,222,220,219,217,215,213,212,210,208,206,204,202,201,199,
 197,195,193,191,189,188,186,184,182,180,178,176,174,172,170,168,166,164,
 162,160,158,156,154,152,150,148,146,144,142,140,138,136,134,132,130,128,
 126,124,121,119,117,115,113,111,109,107,105,102,100, 98, 96, 94, 92, 90,
  87, 85, 83, 81, 79, 77, 74, 72, 70, 68, 66, 64, 61, 59, 57, 55, 53, 50,
  48, 46, 44, 42, 39, 37, 35, 33, 31, 28, 26, 24, 22, 19, 17, 15, 13, 11,
   8,  6,  4,  2 };
/**
 * @brief Transforms data into the DCT domian using a fast precomputed DCT
 * @param input_vector_and_result The input values given in FP representation - Will be replaced by the result vector.
 */
void fct_transform(int16_t *input_vector_and_result)
{
    int16_t result[DCT_COEFF_SIZE] = {0};
    uint16_t n = 0, m = 0;
    int16_t sign = 1;

    for (; m < DCT_COEFF_SIZE; m++) {
        // Prevent watchdog from restarting during this operation
        watchdog_periodic();
        for (n = 0; n < SIGNAL_LEN; n++) {
            sign = INDEX_FORMULA(m,n) / SIGNAL_LEN / 2 % 2 == 0 ? -1 : 1;
            if ((INDEX_FORMULA(m,n) / SIGNAL_LEN) % 2 == 0) {
                result[m] += fp_multiply(input_vector_and_result[n], -sign * c[INDEX_FORMULA(m,n) % SIGNAL_LEN]);
            } else {
                result[m] += fp_multiply(input_vector_and_result[n], sign * c[SIGNAL_LEN - (INDEX_FORMULA(m,n) % SIGNAL_LEN)]);
            }
        }
        // Threshold
        if (result[m] < 0) {
            if (-result[m] < DCT_THRESHOLD) {
                result[m] = 0;
            }
        } else {
            if (result[m] < DCT_THRESHOLD) {
                result[m] = 0;
            }
        }
    }
    memset(input_vector_and_result, 0, BLOCK_LEN);
    memcpy(input_vector_and_result, result, DCT_COEFF_SIZE * sizeof(int16_t));
}

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

huffman_metadata huffman_encode(uint8_t *block_and_result, uint16_t length, const huffman_codeword *codebook, const huffman_codeword h_eof)
{
    huffman_metadata h_data;
    uint16_t i;
    uint8_t firstHalf;
    uint8_t secondHalf;
    uint8_t bitstring[BLOCK_LEN + 1] = {0};
    uint16_t bitstr_len = 0;
    huffman_codeword huff_code1, huff_code2;
    uint8_t remainder;
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
