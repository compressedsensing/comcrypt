#ifndef AES_CBC_H_
#define AES_CBC_H_

#include "contiki.h"

#define ENCRYPT encryption_driver

#include "lib/aes-128.h"


struct encryption_driver {
  
  /**
   * \brief Encrypts using CBC mode. IMPORTANT: plaintext_and_result should be divisable with AES_128_BLOCK_SIZE
   */
  void (* aes_encrypt_cbc)(uint8_t *plaintext_and_result, uint8_t *iv, uint32_t length, uint8_t *key);
};

extern const struct encryption_driver ENCRYPT;

#endif /* AES_CBC_H_ */