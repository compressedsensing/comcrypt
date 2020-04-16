#include "./compression.h"

static void convolution(const int16_t *signal,
                        size_t signal_length,
                        const int16_t *kernel,
                        size_t kernel_length,
                        int16_t *result)
{
    size_t n;

    for (n = 0; n < signal_length + kernel_length - 1; n++)
    {
        size_t kmin, kmax, k;

        result[n] = 0;

        kmin = (n >= kernel_length - 1) ? n - (kernel_length - 1) : 0;
        kmax = (n < signal_length - 1) ? n : signal_length - 1;

        for (k = kmin; k <= kmax; k++)
        {
            result[n] += FP.fp_multiply(signal[k], kernel[n - k]);
        }
    }
}

/** TODO
 * Implement filter2
 * Implement decimation
 * 
 * 
*/

/**
 * @brief Transforms data into the DWT domian 
 * @param input_vector The input values given in FP representation.
 * @param result The result vector
 * @param block_size The size of the block to DCT transform
 */
#define sig_len 512
#define k_len 12
static void dwt_transform(int16_t *input_vector_and_result, unsigned int block_size)
{
    //Define kernel

    // int16_t *output[543] = {};

    /* rbio 5.5*/
    int16_t res[564] = {0};
    int16_t inter[564] = {0};

    //Get convolution coeffecients
    double hn[k_len] = {0, 0.0134567095, -0.0026949669, -0.1367065847,
                        -0.0935046974, 0.4768032658, 0.8995061097, 0.4768032658, -0.0935046974,
                        -0.1367065847, -0.0026949669, 0.0134567095};

    double gn[k_len] = {0, 0.0396870883, -0.0079481086, -0.0544637885, -0.3456052820, 0.7366601814,
                        -0.3456052820, -0.0544637885, -0.0079481086, 0.0396870883,
                        0, 0};

    int16_t bio_filter_h[k_len] = {0};
    int16_t bio_filter_g[k_len] = {0};

    int i;
    for (i = 0; i < k_len; i++)
    {
        bio_filter_g[i] = FP.float_to_fixed16(gn[i]);
        bio_filter_h[i] = FP.float_to_fixed16(hn[i]);
    }

    // //Print kernel
    // for (i = 0; i < k_len; i++)
    // {
    //     printf("%04x\t", my_kernel[i]);
    // }
    // printf("\n");

    //Define test signal
int16_t sig_nal[sig_len] = { 7584,7584,7584,7584,7584,7584,7584,7584,7608,7608,7632,7608,7608,7616,
 7656,7672,7688,7696,7704,7696,7664,7640,7648,7632,7608,7616,7584,7560,
 7568,7576,7560,7544,7520,7512,7488,7488,7464,7464,7464,7480,7512,7512,
 7520,7536,7544,7536,7520,7536,7544,7552,7544,7536,7552,7544,7576,7576,
 7576,7576,7576,7560,7552,7568,7576,7584,7600,7632,7680,7744,7880,8048,
 8280,8568,8896,9224,9536,9800,9912,9880,9608,9160,8520,7784,7016,6360,
 5912,5760,5896,6168,6512,6864,7120,7304,7440,7504,7528,7536,7520,7520,
 7504,7528,7552,7592,7600,7616,7616,7624,7616,7608,7608,7624,7632,7664,
 7704,7736,7744,7776,7752,7760,7744,7744,7736,7744,7744,7744,7760,7760,
 7752,7744,7736,7736,7728,7728,7712,7728,7720,7744,7752,7760,7760,7760,
 7792,7792,7800,7832,7840,7856,7904,7936,7968,8000,8040,8080,8096,8096,
 8128,8152,8160,8184,8200,8216,8240,8272,8280,8288,8272,8256,8224,8232,
 8216,8232,8208,8184,8176,8152,8120,8072,8032,7992,7960,7896,7872,7832,
 7808,7784,7768,7744,7728,7704,7672,7664,7632,7600,7576,7568,7576,7560,
 7560,7568,7568,7568,7560,7552,7552,7536,7528,7512,7528,7536,7552,7568,
 7584,7584,7592,7568,7576,7560,7568,7568,7584,7608,7584,7600,7616,7608,
 7632,7608,7592,7592,7600,7592,7608,7608,7616,7608,7600,7600,7592,7592,
 7592,7568,7568,7560,7560,7560,7560,7560,7560,7568,7544,7528,7504,7504,
 7472,7464,7496,7504,7504,7520,7528,7528,7512,7496,7472,7464,7456,7464,
 7456,7472,7480,7464,7480,7472,7464,7448,7472,7472,7480,7472,7504,7528,
 7544,7568,7576,7600,7600,7624,7632,7640,7616,7616,7600,7592,7560,7560,
 7568,7552,7552,7520,7504,7480,7448,7424,7416,7408,7408,7408,7400,7376,
 7336,7336,7312,7312,7320,7304,7352,7368,7384,7408,7400,7400,7376,7360,
 7344,7336,7328,7336,7344,7360,7368,7392,7392,7400,7384,7384,7376,7376,
 7368,7368,7392,7424,7472,7576,7720,7936,8160,8504,8840,9192,9528,9800,
 9976,9984,9792,9376,8760,7992,7168,6368,5784,5472,5496,5760,6144,6560,
 6912,7232,7392,7528,7552,7560,7552,7528,7512,7488,7528,7568,7584,7624,
 7632,7656,7664,7672,7688,7728,7728,7760,7808,7824,7864,7896,7912,7944,
 7968,7968,7976,7968,7976,7968,7976,7984,7992,8008,8016,8016,8016,8016,
 8000,8000,8016,8032,8072,8088,8128,8152,8176,8208,8216,8232,8240,8264,
 8280,8320,8360,8384,8408,8432,8456,8448,8440,8448,8464,8464,8448,8472,
 8480,8464,8464,8488,8488,8480,8480,8448,8440,8416,8400,8384,8368,8328,
 8320,8264,8256,8240,8176,8112,8072,8008,7960,7912,7872,7864,7840,7832,
 7808,7776,7736,7696,7656,7624,7608,7592,7584,7592,7584,7576,7576,7568,
 7576,7544,7528,7536,7520,7528,7536,7520,7528,7552,7568,7552,7552,7528,
 7536,7528,7528,7536,7544,7544,7568,7576,7592,7616,7616,7600,7592,7592,
 7584,7576,7584,7600,7608,7624,7616,7616 };
    // printf("Signal :\n");
    // for (i = 0; i < sig_len; i++)
    // {
    //     printf("%04x\t", sig_nal[i]);
    // }
    // printf("\n");

    //Test convolution
    convolution(sig_nal, sig_len, bio_filter_g, k_len, res);

    // int j = 0;
    for (i = 1; i < k_len + sig_len - 1; i += 2)
    {
#if FLOAT
        printf("%.4f,", FP.fixed_to_float16(res[i]));
#elif
        printf("%04x\t", res[i]);
#endif

        // printf("%d\t", j);
        // j++;
        /* code */
    }

    // printf("\n\n");

    convolution(sig_nal, sig_len, bio_filter_h, k_len, res);
    convolution(res, 261, bio_filter_h, k_len, inter);

    convolution(inter, 261, bio_filter_g, k_len, res);

    for (i = 1; i < 136 * 2; i += 2)
    {
        // j++;
#if FLOAT
        printf("%.4f,", FP.fixed_to_float16(res[i]));
#elif
        printf("%04x\t", res[i]);
#endif

        // printf("%d\t", j);
        // j++;
        /* code */
    }
    // printf("\n\n");
    // printf("\n%d\n",j);

    convolution(inter, 261, bio_filter_h, k_len, res);
    convolution(res, 136, bio_filter_h, k_len, inter);

    convolution(inter, 136, bio_filter_g, k_len, res);

    for (i = 1; i < 73 * 2; i += 2)
    {
#if FLOAT
        printf("%.4f,", FP.fixed_to_float16(res[i]));
#elif
        printf("%04x\t", res[i]);
#endif

        // printf("%d\t", j);
        // j++;
        /* code */
    }

    // printf("\n\n");
    convolution(inter, sig_len / 4, bio_filter_h, k_len, res);
    for (i = 1; i < 73 * 2; i += 2)
    {
#if FLOAT
        printf("%.4f,", FP.fixed_to_float16(res[i]));
#elif
        printf("%04x\t", res[i]);
#endif

        // printf("%d\t", j);
        // j++;
        /* code */
    }

    printf("\n");

    //     printf("Result :\n");
    //     for (i = 0; i < (k_len + sig_len - 1); i++)
    //     {
    // #if FLOAT
    //         printf("%.4f,", FP.fixed_to_float16(res[i]));
    // #elif
    //         printf("%04x\t", res[i]);
    // #endif
    //     }
    //     printf("\n");
}

static const int16_t c[SIGNAL_LEN] = {181, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
                                      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 254, 254, 254, 254,
                                      254, 254, 254, 254, 254, 253, 253, 253, 253, 253, 253, 253, 253, 253, 252, 252, 252, 252,
                                      252, 252, 252, 252, 251, 251, 251, 251, 251, 251, 251, 250, 250, 250, 250, 250, 250, 249,
                                      249, 249, 249, 249, 249, 248, 248, 248, 248, 248, 247, 247, 247, 247, 247, 246, 246, 246,
                                      246, 246, 245, 245, 245, 245, 244, 244, 244, 244, 244, 243, 243, 243, 243, 242, 242, 242,
                                      242, 241, 241, 241, 241, 240, 240, 240, 239, 239, 239, 239, 238, 238, 238, 237, 237, 237,
                                      237, 236, 236, 236, 235, 235, 235, 234, 234, 234, 234, 233, 233, 233, 232, 232, 232, 231,
                                      231, 231, 230, 230, 230, 229, 229, 229, 228, 228, 227, 227, 227, 226, 226, 226, 225, 225,
                                      225, 224, 224, 223, 223, 223, 222, 222, 221, 221, 221, 220, 220, 219, 219, 219, 218, 218,
                                      217, 217, 217, 216, 216, 215, 215, 215, 214, 214, 213, 213, 212, 212, 211, 211, 211, 210,
                                      210, 209, 209, 208, 208, 207, 207, 207, 206, 206, 205, 205, 204, 204, 203, 203, 202, 202,
                                      201, 201, 200, 200, 199, 199, 198, 198, 197, 197, 196, 196, 195, 195, 194, 194, 193, 193,
                                      192, 192, 191, 191, 190, 190, 189, 189, 188, 188, 187, 187, 186, 185, 185, 184, 184, 183,
                                      183, 182, 182, 181, 181, 180, 179, 179, 178, 178, 177, 177, 176, 175, 175, 174, 174, 173,
                                      173, 172, 171, 171, 170, 170, 169, 168, 168, 167, 167, 166, 166, 165, 164, 164, 163, 163,
                                      162, 161, 161, 160, 159, 159, 158, 158, 157, 156, 156, 155, 155, 154, 153, 153, 152, 151,
                                      151, 150, 149, 149, 148, 148, 147, 146, 146, 145, 144, 144, 143, 142, 142, 141, 140, 140,
                                      139, 138, 138, 137, 136, 136, 135, 134, 134, 133, 132, 132, 131, 130, 130, 129, 128, 128,
                                      127, 126, 126, 125, 124, 124, 123, 122, 122, 121, 120, 119, 119, 118, 117, 117, 116, 115,
                                      115, 114, 113, 112, 112, 111, 110, 110, 109, 108, 108, 107, 106, 105, 105, 104, 103, 103,
                                      102, 101, 100, 100, 99, 98, 97, 97, 96, 95, 95, 94, 93, 92, 92, 91, 90, 89,
                                      89, 88, 87, 86, 86, 85, 84, 84, 83, 82, 81, 81, 80, 79, 78, 78, 77, 76,
                                      75, 75, 74, 73, 72, 72, 71, 70, 69, 69, 68, 67, 66, 66, 65, 64, 63, 62,
                                      62, 61, 60, 59, 59, 58, 57, 56, 56, 55, 54, 53, 53, 52, 51, 50, 49, 49,
                                      48, 47, 46, 46, 45, 44, 43, 42, 42, 41, 40, 39, 39, 38, 37, 36, 36, 35,
                                      34, 33, 32, 32, 31, 30, 29, 28, 28, 27, 26, 25, 25, 24, 23, 22, 21, 21,
                                      20, 19, 18, 18, 17, 16, 15, 14, 14, 13, 12, 11, 10, 10, 9, 8, 7, 7,
                                      6, 5, 4, 3, 3, 2, 1, 0};

/**
 * @brief Transforms data into the DCT domian using a fast precomputed DCT
 * @param input_vector_and_result The input values given in FP representation - Will be replaced by the result vector.
 */
static void fct(int16_t *input_vector_and_result)
{
    int16_t result[DCT_COEFF_SIZE] = {0};
    uint16_t n = 0, m = 0;
    int16_t sign = 1;

    for (; m < DCT_COEFF_SIZE; m++)
    {
        // Prevent watchdog from restarting during this operation
        watchdog_periodic();
        for (n = 0; n < SIGNAL_LEN; n++)
        {
            sign = INDEX_FORMULA(m, n) / SIGNAL_LEN / 2 % 2 == 0 ? -1 : 1;
            if ((INDEX_FORMULA(m, n) / SIGNAL_LEN) % 2 == 0)
            {
                result[m] += FP.fp_multiply(input_vector_and_result[n], -sign * c[INDEX_FORMULA(m, n) % SIGNAL_LEN]);
            }
            else
            {
                result[m] += FP.fp_multiply(input_vector_and_result[n], sign * c[SIGNAL_LEN - (INDEX_FORMULA(m, n) % SIGNAL_LEN)]);
            }
        }
    }
    memset(input_vector_and_result, 0, BLOCK_LEN);
    memcpy(input_vector_and_result, result, DCT_COEFF_SIZE * sizeof(int16_t));
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
const struct compression_driver compression_driver = {
    dwt_transform,
    //dct_transform,
    threshold,
    // simple_truncate,
    huffman_encode,
    fct};
