#include "./compression.h"

static void convolution(const int16_t *signal,
                        size_t signal_length,
                        const int16_t *kernel,
                        size_t kernel_length,
                        int16_t *result)
{
    size_t n;

    for (n = 1; n < signal_length + kernel_length - 1; n += 2)
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
#define DWT_FILTER_SIZE 12
static uint16_t dwt_transform(int16_t *input_vector_and_result, unsigned int block_size)
{
    //Define kernel

    // int16_t *output[543] = {};
    // int16_t sig_nal[sig_len] = {948, 948, 948, 948, 948, 948, 948, 948, 951, 951, 954, 951, 951, 952,
    //                             957, 959, 961, 962, 963, 962, 958, 955, 956, 954, 951, 952, 948, 945,
    //                             946, 947, 945, 943, 940, 939, 936, 936, 933, 933, 933, 935, 939, 939,
    //                             940, 942, 943, 942, 940, 942, 943, 944, 943, 942, 944, 943, 947, 947,
    //                             947, 947, 947, 945, 944, 946, 947, 948, 950, 954, 960, 968, 985, 1006,
    //                             1035, 1071, 1112, 1153, 1192, 1225, 1239, 1235, 1201, 1145, 1065, 973, 877, 795,
    //                             739, 720, 737, 771, 814, 858, 890, 913, 930, 938, 941, 942, 940, 940,
    //                             938, 941, 944, 949, 950, 952, 952, 953, 952, 951, 951, 953, 954, 958,
    //                             963, 967, 968, 972, 969, 970, 968, 968, 967, 968, 968, 968, 970, 970,
    //                             969, 968, 967, 967, 966, 966, 964, 966, 965, 968, 969, 970, 970, 970,
    //                             974, 974, 975, 979, 980, 982, 988, 992, 996, 1000, 1005, 1010, 1012, 1012,
    //                             1016, 1019, 1020, 1023, 1025, 1027, 1030, 1034, 1035, 1036, 1034, 1032, 1028, 1029,
    //                             1027, 1029, 1026, 1023, 1022, 1019, 1015, 1009, 1004, 999, 995, 987, 984, 979,
    //                             976, 973, 971, 968, 966, 963, 959, 958, 954, 950, 947, 946, 947, 945,
    //                             945, 946, 946, 946, 945, 944, 944, 942, 941, 939, 941, 942, 944, 946,
    //                             948, 948, 949, 946, 947, 945, 946, 946, 948, 951, 948, 950, 952, 951,
    //                             954, 951, 949, 949, 950, 949, 951, 951, 952, 951, 950, 950, 949, 949,
    //                             949, 946, 946, 945, 945, 945, 945, 945, 945, 946, 943, 941, 938, 938,
    //                             934, 933, 937, 938, 938, 940, 941, 941, 939, 937, 934, 933, 932, 933,
    //                             932, 934, 935, 933, 935, 934, 933, 931, 934, 934, 935, 934, 938, 941,
    //                             943, 946, 947, 950, 950, 953, 954, 955, 952, 952, 950, 949, 945, 945,
    //                             946, 944, 944, 940, 938, 935, 931, 928, 927, 926, 926, 926, 925, 922,
    //                             917, 917, 914, 914, 915, 913, 919, 921, 923, 926, 925, 925, 922, 920,
    //                             918, 917, 916, 917, 918, 920, 921, 924, 924, 925, 923, 923, 922, 922,
    //                             921, 921, 924, 928, 934, 947, 965, 992, 1020, 1063, 1105, 1149, 1191, 1225,
    //                             1247, 1248, 1224, 1172, 1095, 999, 896, 796, 723, 684, 687, 720, 768, 820,
    //                             864, 904, 924, 941, 944, 945, 944, 941, 939, 936, 941, 946, 948, 953,
    //                             954, 957, 958, 959, 961, 966, 966, 970, 976, 978, 983, 987, 989, 993,
    //                             996, 996, 997, 996, 997, 996, 997, 998, 999, 1001, 1002, 1002, 1002, 1002,
    //                             1000, 1000, 1002, 1004, 1009, 1011, 1016, 1019, 1022, 1026, 1027, 1029, 1030, 1033,
    //                             1035, 1040, 1045, 1048, 1051, 1054, 1057, 1056, 1055, 1056, 1058, 1058, 1056, 1059,
    //                             1060, 1058, 1058, 1061, 1061, 1060, 1060, 1056, 1055, 1052, 1050, 1048, 1046, 1041,
    //                             1040, 1033, 1032, 1030, 1022, 1014, 1009, 1001, 995, 989, 984, 983, 980, 979,
    //                             976, 972, 967, 962, 957, 953, 951, 949, 948, 949, 948, 947, 947, 946,
    //                             947, 943, 941, 942, 940, 941, 942, 940, 941, 944, 946, 944, 944, 941,
    //                             942, 941, 941, 942, 943, 943, 946, 947, 949, 952, 952, 950, 949, 949,
    //                             948, 947, 948, 950, 951, 953, 952, 952};

    /* rbio 5.5*/
    int16_t res[564] = {0};
    int16_t inter[564] = {0};
    int16_t output[564] = {0};
    uint8_t binary_map[68] = {0};
    uint16_t output_length = 0;
    // uint16_t binary_map_index_counter = 0;

    int i, j, ii;

    const int16_t bio_filter_h[DWT_FILTER_SIZE] = {0, 55, -11, -559, -382, 1952, 3684, 1952, -382, -559, -11, 55};
    const int16_t bio_filter_g[DWT_FILTER_SIZE] = {0, 162, -32, -223, -1415, 3017, -1415, -223, -32, 162, 0, 0};

    ii = 0;

    //Test convolution
    convolution(input_vector_and_result, sig_len, bio_filter_g, k_len, res);

    for (i = 1; i < k_len + sig_len - 1; i += 2)
    {
        output[ii] = res[i];
        ii++;
    }

    convolution(input_vector_and_result, sig_len, bio_filter_h, k_len, res);

    //Decimate
    j = 0;
    for (i = 1; i < k_len + sig_len - 1; i += 2)
    {
        inter[j] = res[i];
        j++;
    }

    convolution(inter, 261 + 11, bio_filter_g, k_len, res);

    for (i = 1; i < (136 * 2); i += 2)
    {
        output[ii] = res[i];
        ii++;
    }

    convolution(inter, 261 + 11, bio_filter_h, k_len, res);

    j = 0;
    for (i = 1; i < 261 + 11; i += 2)
    {
        inter[j] = res[i];
        j++;
    }

    convolution(inter, 136 + 11, bio_filter_g, k_len, res);

    for (i = 1; i < 136 + 11; i += 2)
    {
        output[ii] = res[i];
        ii++;
    }

    convolution(inter, 136 + 11, bio_filter_h, k_len, res);

    for (i = 1; i < 136 + 11; i += 2)
    {
        output[ii] = res[i];
        ii++;
    }



    // printf("\nCorrect DWT coef : \n");
    // for (i = 0; i < ii; i ++)
    // {
    //     printf("%d\t", output[i]);
    // }
    // printf("\n\n");
    /* THRESHOLD AND BINARY MAP */
    int16_t threshold = 50;

    for (i = 0; i < 543; i++)
    {
        uint8_t bin = 1;

        if (output[i] < 0) /* Check if negative*/
        {
            if (-output[i] < threshold)
            {
                output[i] = 0;
                bin = 0;
            }
        }
        else
        {
            if (output[i] < threshold)
            {
                output[i] = 0;
                bin = 0;
            }
        }

        /* Append binary index correctly */
        binary_map[i / 8] |= (bin << (8 - (i % 8) - 1));
        
        /* Add signal to output */
        if (bin)
        {
            output[output_length] = output[i];
            output_length++;
        }
    }


    /* Find quantization coefficients*/
    int16_t min_val,max_val = 0;

    for (i = 0; i < output_length; i++)
    {
        if(output[i] < min_val)
        {
            min_val = output[i];
        }
        if(output[i] > max_val)
        {
            max_val = output[i];
        }
    }
    


    // printf("\nBinary Map : \n");
    // for (i = 0; i < 68; i++)
    // {
    //     printf("%02x", (uint8_t)binary_map[i]);
    // }

    // printf("\n");
    // for (i = 0; i < output_length; i++)
    // {
    //     // printf("%04x", (uint16_t)signal[i]);
    //     // printf("%d,", input_vector_and_result[i]);
    // }
    
    /* Assign binary map to output*/
    for (i = 0; i < 34; i++)
    {
        input_vector_and_result[i] = (binary_map[(2*i)] << 8) | binary_map[(2*i)+1];
    }
    // for (i = 0; i < 34; i++)
    // {
    //     printf("%04x", (uint16_t)input_vector_and_result[i]);
    // }
    
    for (i = 0; i < output_length; i++)
    {
        input_vector_and_result[i+34] = output[i];
    }
    
    // printf("\n\n");
    // printf("%d", output_length);
    // printf("\n\n");

    // for (i = 0; i < 68; i++)
    // {
    //     /* code */
    // }
    
    // for (i = 0; i < output_length; i++)
    // {
    //     input_vector_and_result[i+68] = output[i];
    // }

    for (i = output_length+34; i < ii; i++)
    {
        input_vector_and_result[i] = 0;
    }
    // printf("\n\n");

    return output_length+34;

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
