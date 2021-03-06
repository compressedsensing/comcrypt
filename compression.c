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

static const int16_t c[SIGNAL_LEN] = { 181,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
 255,255,255,255,255,255,255,255,255,255,255,254,254,254,254,254,254,254,
 254,254,254,254,254,253,253,253,253,253,253,253,253,253,252,252,252,252,
 252,252,252,252,251,251,251,251,251,251,251,250,250,250,250,250,250,249,
 249,249,249,249,249,248,248,248,248,248,247,247,247,247,247,246,246,246,
 246,246,245,245,245,245,244,244,244,244,244,243,243,243,243,242,242,242,
 242,241,241,241,241,240,240,240,239,239,239,239,238,238,238,237,237,237,
 237,236,236,236,235,235,235,234,234,234,234,233,233,233,232,232,232,231,
 231,231,230,230,230,229,229,229,228,228,227,227,227,226,226,226,225,225,
 225,224,224,223,223,223,222,222,221,221,221,220,220,219,219,219,218,218,
 217,217,217,216,216,215,215,215,214,214,213,213,212,212,211,211,211,210,
 210,209,209,208,208,207,207,207,206,206,205,205,204,204,203,203,202,202,
 201,201,200,200,199,199,198,198,197,197,196,196,195,195,194,194,193,193,
 192,192,191,191,190,190,189,189,188,188,187,187,186,185,185,184,184,183,
 183,182,182,181,181,180,179,179,178,178,177,177,176,175,175,174,174,173,
 173,172,171,171,170,170,169,168,168,167,167,166,166,165,164,164,163,163,
 162,161,161,160,159,159,158,158,157,156,156,155,155,154,153,153,152,151,
 151,150,149,149,148,148,147,146,146,145,144,144,143,142,142,141,140,140,
 139,138,138,137,136,136,135,134,134,133,132,132,131,130,130,129,128,128,
 127,126,126,125,124,124,123,122,122,121,120,119,119,118,117,117,116,115,
 115,114,113,112,112,111,110,110,109,108,108,107,106,105,105,104,103,103,
 102,101,100,100, 99, 98, 97, 97, 96, 95, 95, 94, 93, 92, 92, 91, 90, 89,
  89, 88, 87, 86, 86, 85, 84, 84, 83, 82, 81, 81, 80, 79, 78, 78, 77, 76,
  75, 75, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 66, 66, 65, 64, 63, 62,
  62, 61, 60, 59, 59, 58, 57, 56, 56, 55, 54, 53, 53, 52, 51, 50, 49, 49,
  48, 47, 46, 46, 45, 44, 43, 42, 42, 41, 40, 39, 39, 38, 37, 36, 36, 35,
  34, 33, 32, 32, 31, 30, 29, 28, 28, 27, 26, 25, 25, 24, 23, 22, 21, 21,
  20, 19, 18, 18, 17, 16, 15, 14, 14, 13, 12, 11, 10, 10,  9,  8,  7,  7,
   6,  5,  4,  3,  3,  2,  1,  0 };

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
