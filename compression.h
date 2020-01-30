#ifndef COMPRESSION_H_
#define COMPRESSION_H_

#include "contiki.h"
#include "math.h"
#include <stddef.h>

#define COMPRESS compression_driver

struct compression_driver {
  
  /**
   * \brief Encrypts using CBC mode. IMPORTANT: plaintext_and_result should be divisable with AES_128_BLOCK_SIZE
   */
  void (* dct_transform)(float* input_vector,float* res, unsigned int block_size);

  // void (* aes_encrypt_cbc)(uint8_t *plaintext_and_result, uint8_t *iv, uint32_t length, uint8_t *key);
};



extern const struct compression_driver COMPRESS;

#endif /* DCT_H_ */