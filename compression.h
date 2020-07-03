#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "contiki.h"
#include "./configuration.h"
#include "string.h"
#include "./fixedpoint.h"
#include "sys/log.h"

#define DEFAULT_HUFFMAN_CODEBOOK default_huffman_codebook
#define DEFAULT_HUFFMAN_EOF default_huffman_eof

#define HUFFMAN_RESOLUTION 4 // Amount of bits to represent symbols
#define HUFFMAN_BLOCK_MAX_SIZE 170
#define INDEX_FORMULA(m,n) (m*(n*2+1))
#define DCT_COEFF_SIZE 130
#define DCT_THRESHOLD 0x00000000

struct huffman_metadata {
  uint16_t length;
  uint16_t byte_length;
  uint8_t *bits;
  int8_t success;
};

struct huffman_codeword {
  uint16_t word;
  uint8_t word_length;
};

typedef struct {
  uint16_t word;
  uint8_t word_length;
} huffman_codeword;

typedef struct {
  uint16_t length;
  uint16_t byte_length;
  int8_t success;
} huffman_metadata;

/**
 * @brief Transforms data into the DWT domian 
 * @param input_vector_and_result The input values given in FP representation, and the result.
*/
uint16_t dwt_transform(int16_t *input_vector_and_result);
/**
 * @brief Transforms data into via the DCT-II transform using a naive and slow approach.
 * https://en.wikipedia.org/wiki/Discrete_cosine_transform#DCT-II
*/
void dct_transform(int16_t *input_vector_and_result, unsigned int block_size);

/**
 * \brief Huffman encodes based on the codebook defined in HUFFMAN_CODEBOOK
*/
huffman_metadata huffman_encode(uint8_t *block, uint16_t length, const huffman_codeword *codebook, const huffman_codeword h_eof);

/**
  @brief Transforms data into the DCT domian using a fast precomputed DCT
  @param input_vector_and_result The input values given in FP representation - Will be replaced by the result vector.
*/
void fct_transform(int16_t *input_vector_and_result);


#endif /* COMPRESSION_H_ */