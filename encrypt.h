#ifndef ENCRYPTION_H_
#define ENCRYPTION_H_

#include "contiki.h"

#define ENCRYPT encryption_driver

#include "lib/aes-128.h"
#include "string.h"


struct encryption_driver {
  /**
   * \brief Encrypts using CTR mode. IMPORTANT: plaintext_and_result should be divisable with AES_128_BLOCK_SIZE
   */
  void (* aes_encrypt_ctr)(uint8_t *plaintext_and_result, uint8_t *iv, const uint32_t length, const uint8_t *key);
};

extern const struct encryption_driver ENCRYPT;

#endif /* ENCRYPTION_H_ */