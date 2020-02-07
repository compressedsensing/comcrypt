#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "contiki.h"
#include "math.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HUFFMAN_CODEBOOK
#define CODEBOOK           HUFFMAN_CODEBOOK
#else /* HUFFMAN_CODEBOOK */
#define CODEBOOK            huffman_codebook
#endif /* HUFFMAN_CODEBOOK */

#define COMPRESS compression_driver

#define HUFFMAN_RESOLUTION 4 // Amount of bits to represent symbols

struct huffman_data {
  uint16_t length;
  uint8_t *bits;
};

struct compression_driver {
  
  /**
   * \brief Transforms data into via the DCT-II transform.
   * https://en.wikipedia.org/wiki/Discrete_cosine_transform#DCT-II
   */
  void (* dct_transform)(float* input_vector,float* res, unsigned int block_size);
  /**
   * \brief Huffman encodes based on the codebook defined in HUFFMAN_CODEBOOK
   */
  int8_t (* huffman_encode)(uint8_t *block, uint16_t length, struct huffman_data *h_data);

};

// Huffman variables

// 4 bit resolution
const uint16_t huffman_codebook[1 << HUFFMAN_RESOLUTION];


extern const struct compression_driver COMPRESS;

#endif /* DCT_H_ */