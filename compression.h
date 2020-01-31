#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "contiki.h"
#include "math.h"
#include <stddef.h>

#define COMPRESS compression_driver

struct compression_driver {
  
  /**
   * \brief Transforms data into via the DCT-II transform.
   * https://en.wikipedia.org/wiki/Discrete_cosine_transform#DCT-II
   */
  void (* dct_transform)(float* input_vector,float* res, unsigned int block_size);

};



extern const struct compression_driver COMPRESS;

#endif /* DCT_H_ */