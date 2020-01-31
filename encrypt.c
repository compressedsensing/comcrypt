#include "./encrypt.h"

void pkcs7_pad(uint8_t *block, uint8_t block_length) {
  // do nothing at the moment - not sure if it is neccesary for our purpose
}

void xor_blocks(uint8_t *block_1_and_result, uint8_t *block_2, uint8_t block_size) {
  uint8_t i;
  for (i = 0; i < block_size; i++) {
    block_1_and_result[i] ^= block_2[i];
  }
}

void copy_block(uint8_t *to, uint8_t *from, uint8_t block_length) {
  uint8_t i;
  for (i = 0; i < block_length; i++) {
    to[i] = from[i];
  }
}

void incrementCtr(uint8_t *iv_and_result, uint8_t block_size) {
  unsigned int bi;
  for (bi = (block_size - 1); bi >= 0; --bi) {
	/* inc will overflow */
    if (iv_and_result[bi] == 255) {
      iv_and_result[bi] = 0;
      continue;
    } 
    iv_and_result[bi] += 1;
    break;   
  }
}

// Reference: https://en.wikipedia.org/wiki/Block_cipher_mode_of_operation
static void aes_encrypt_cbc(uint8_t *plaintext_and_result, uint8_t *iv, uint32_t length, uint8_t *key) {
  const uint8_t block_size = AES_128_BLOCK_SIZE;
  uint8_t i;

  AES_128.set_key(key);

  uint32_t blocks_length = length / block_size;
  const uint8_t remainder = length % block_size;
  // Checking if last block is the size of "block_size"
  if (remainder > 0) {
    // If it is not pad last block so it fits within "block_size"
    blocks_length = blocks_length + 1;
    pkcs7_pad(plaintext_and_result + blocks_length * block_size, remainder);
  }

  // XOR first block with initialization vector
  xor_blocks(plaintext_and_result, iv, block_size);
  AES_128.encrypt(plaintext_and_result);

  // XOR each consecutive block with the previous block
  for (i = 1; i < blocks_length; i++) {
    xor_blocks(plaintext_and_result + i * block_size, plaintext_and_result + (i - 1) * block_size, block_size);
    AES_128.encrypt(plaintext_and_result + i * block_size);
  }
  
}

static void aes_encrypt_ctr(uint8_t *plaintext_and_result, uint8_t *iv, uint32_t length, uint8_t *key) {
  const uint8_t block_size = AES_128_BLOCK_SIZE;
  uint8_t i;
  uint8_t tempIv[block_size];

  AES_128.set_key(key);

  uint32_t blocks_length = length / block_size;
  const uint8_t remainder = length % block_size;
  // Checking if last block is the size of "block_size"
  if (remainder > 0) {
    // If it is not pad last block so it fits within "block_size"
    blocks_length = blocks_length + 1;
    pkcs7_pad(plaintext_and_result + blocks_length * block_size, remainder);
  }

  // For each block in plaintext
  for (i = 0; i < blocks_length; i++) {
    // Increment IV and encrypt it
    copy_block(tempIv, iv, block_size);
    AES_128.encrypt(tempIv);
    incrementCtr(iv, block_size);
    // XOR it with the plaintext block
    xor_blocks(plaintext_and_result + i * block_size, tempIv, block_size);
  } 
}

const struct encryption_driver encryption_driver = {
  aes_encrypt_cbc,
  aes_encrypt_ctr
};