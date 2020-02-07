#include "./compression.h"

const uint16_t huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
  0x0A98, 0x0152, 0x015E, 0x00A8, 
  0x0055, 0x0028, 0x000B, 0x0000, 
  0x0003, 0x0004, 0x0029, 0x0056, 
  0x00AE, 0x015F, 0x02A7, 0x054D, 
};

const uint16_t huffman_eof = 0xA99;

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
    uint16_t sig_bit = 1 << (max_code_word_size - 1);
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
                bitstr_len++;
                last_byte_bit = bitstr_len % 8;
            }
            return bitstr_len;
        }
    }
    // If none return 1 - there should be a single zero which there already is
    return bitstr_len + 1;
}

static int8_t huffman_encode(uint8_t *block, uint16_t length, struct huffman_data *h_data) {
    uint16_t i;
    uint8_t firstHalf;
    uint8_t secondHalf;
    uint8_t *bitstring;
    uint16_t last_bit_position = 0;
    uint16_t huff_code;
    uint8_t *buf;
    uint8_t remainder = 0;
    bitstring = calloc(length * 2, sizeof(uint8_t)); // Potentially N = 512 Bytes
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

    // When done push eof
    last_bit_position = pushBits(huffman_eof, bitstring, last_bit_position);

    if (last_bit_position % 8) {
        remainder = 1;
    }
    buf = calloc((last_bit_position >> 3) + remainder, sizeof(uint8_t));
    memcpy(buf, bitstring, (last_bit_position >> 3) + remainder);
    free(bitstring);
    h_data->length = last_bit_position;
    h_data->bits = buf;

    if ((h_data->length >> 3) > length) {
        return -1;
    } else {
        return 1;
    }
} 

const struct compression_driver compression_driver = {
    dct_transform,
    huffman_encode
};