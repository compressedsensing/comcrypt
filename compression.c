#include "./compression.h"

const uint16_t huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
  0x4000, 0x4001, 0x2001, 0x1001, 
  0x801, 0x401, 0x9, 0x0, 
  0x3, 0x5, 0x201, 0x101, 
  0x81, 0x41, 0x21, 0x11, 
};

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
    uint16_t max_code_word_size = sizeof(uint16_t) * 8;
    uint16_t sig_bit = 1 << (max_code_word_size - 1); // Might overflow
    uint8_t i, j;
    uint8_t last_byte_bit = bitstring_length % 8;
    uint16_t bitstr_len = bitstring_length;
    uint16_t start;
    uint8_t bit;
    for (i = 0; i < max_code_word_size; i++) {
        start = (sig_bit >> i) & huff_code;
        if (start) {
            // we have arrived at the index of the starting bit
            // Append it to bitstring at bitstring_length
            for (j = 0; j < max_code_word_size - i; i++) {
                bit = (((huff_code << (i + j)) & sig_bit) >> last_byte_bit) >> 8;
                bitstring[bitstr_len >> 3] |= bit;
                // memcpy(&bitstring[bitstr_len >> 4], &bit, 1);
                bitstr_len++;
                last_byte_bit = bitstr_len % 8;
            }
            // printf("%02x\t", bitstring[bitstring_length >> 3]);
            // printf("%02x\n", bitstring[(bitstring_length-1) >> 3]);
            return bitstr_len;
        }
        // Find first 1 from most significant bit
        // From that first 1 to the least significant bit - or that with the 
        // bitstring from length and i - sizeof(uint16_t) * 8 forward
        // Return i - sizeof(uint16_t) * 8
    }
    // If none return 1 - there should be a single zero which there already is
    return 1;
}

static void huffman_encode(uint8_t *block, uint16_t length, struct huffman_data *h_data) {
    uint16_t i;
    uint8_t firstHalf;
    uint8_t secondHalf;
    uint8_t *bitstring;
    uint16_t last_bit_position = 0;
    uint16_t huff_code;
    bitstring = calloc(length, sizeof(uint8_t)); // Potentially N = 512 Bytes
    for (i = 0; i < length; i++) {
        // Push first half to final huff code
        firstHalf = (block[i] & 0xF0) >> 4;
        huff_code = CODEBOOK[firstHalf];
        last_bit_position = pushBits(huff_code, bitstring, last_bit_position);

        // Repeat for second half
        secondHalf = block[i] & 0x0F;
        huff_code = CODEBOOK[secondHalf];
        last_bit_position = pushBits(huff_code, bitstring, last_bit_position);
    }
    h_data->length = last_bit_position;
    h_data->bits = bitstring;
} 

const struct compression_driver compression_driver = {
    dct_transform,
    huffman_encode
};