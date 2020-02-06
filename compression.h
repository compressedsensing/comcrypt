#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "contiki.h"
#include "math.h"
#include <stddef.h>

#ifdef HUFFMAN_CODEBOOK
#define CODEBOOK           HUFFMAN_CODEBOOK
#else /* HUFFMAN_CODEBOOK */
#define CODEBOOK            huffman_codebook
#endif /* HUFFMAN_CODEBOOK */

#define COMPRESS compression_driver

#define HUFFMAN_RESOLUTION 4 // Amount of bits to represent symbols

struct compression_driver {
  
  /**
   * \brief Transforms data into via the DCT-II transform.
   * https://en.wikipedia.org/wiki/Discrete_cosine_transform#DCT-II
   */
  void (* dct_transform)(float* input_vector,float* res, unsigned int block_size);

};

// Huffman variables

// 4 bit resolution
extern const uint16_t huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
  0x4000, 0x4001, 0x2001, 0x1001, 
  0x801, 0x401, 0x9, 0x0, 
  0x3, 0x5, 0x201, 0x101, 
  0x81, 0x41, 0x21, 0x11, 
};

struct huffman_data {
  uint16_t length;
  uint8_t *bits;
};



extern const struct compression_driver COMPRESS;

#endif /* DCT_H_ */