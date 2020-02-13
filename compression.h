#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "contiki.h"
#include <stddef.h>
#include <stdlib.h>
#include "./fixedpoint.h"

#define COMPRESS compression_driver

#define DEFAULT_HUFFMAN_CODEBOOK default_huffman_codebook
#define DEFAULT_HUFFMAN_EOF default_huffman_eof

#define COMPRESS compression_driver

#define HUFFMAN_RESOLUTION 4 // Amount of bits to represent symbols
#define HUFFMAN_BLOCK_MAX_SIZE 170

struct huffman_data {
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
  uint8_t *bits;
  int8_t success;
} huffman_data;

struct compression_driver {
  
  /**
   * @brief Transforms data into via the DCT-II transform.
   * https://en.wikipedia.org/wiki/Discrete_cosine_transform#DCT-II
   */
  void (*dct_transform)(FIXED11_21 *input_vector, FIXED11_21 *result, unsigned int block_size);
  
  /**
   * @brief Thresholds the DCT vector.
   */
  void (*threshold)(FIXED11_21 *dct_vector, FIXED11_21 threshold, unsigned int length);

  void (*simple_truncate)(FIXED11_21 *dct_vector,FIXED11_21 *result, uint16_t length, uint16_t result_length);
  // void (* dct_transform)(float* input_vector,float* res, unsigned int block_size);
  /**
   * \brief Huffman encodes based on the codebook defined in HUFFMAN_CODEBOOK
   */
  huffman_data (* huffman_encode)(uint8_t *block, uint16_t length, const huffman_codeword *codebook, const huffman_codeword h_eof);

};

// Huffman variables

// 4 bit resolution
const huffman_codeword default_huffman_codebook[1 << HUFFMAN_RESOLUTION];

const huffman_codeword default_huffman_eof;



extern const struct compression_driver COMPRESS;

#endif /* COMPRESSION_H_ */