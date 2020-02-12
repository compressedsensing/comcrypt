#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "contiki.h"
// #include "math.h"
// #include <stddef.h>
#include <stdlib.h>

#define DEFAULT_HUFFMAN_CODEBOOK default_huffman_codebook
#define DEFAULT_HUFFMAN_EOF default_huffman_eof

#define COMPRESS compression_driver

#define HUFFMAN_RESOLUTION 4 // Amount of bits to represent symbols
#define HUFFMAN_BLOCK_MAX_SIZE 170

struct huffman_data {
  uint16_t length;
  uint8_t *bits;
  uint8_t success;
};

struct compression_driver {
  
  /**
   * \brief Transforms data into via the DCT-II transform.
   * https://en.wikipedia.org/wiki/Discrete_cosine_transform#DCT-II
   */
  // void (* dct_transform)(float* input_vector,float* res, unsigned int block_size);
  /**
   * \brief Huffman encodes based on the codebook defined in HUFFMAN_CODEBOOK
   */
  struct huffman_data (* huffman_encode)(uint8_t *block, uint16_t length, const uint16_t *codebook, const uint16_t h_eof);

};

// Huffman variables

// 4 bit resolution
const uint16_t default_huffman_codebook[1 << HUFFMAN_RESOLUTION];

const uint16_t default_huffman_eof;


extern const struct compression_driver COMPRESS;

#endif /* DCT_H_ */