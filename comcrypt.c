#include "contiki.h"
#include <stdio.h>
#include "lib/aes-128.h"

#define PLAIN_TEXT_SIZE        AES_128_BLOCK_SIZE * 3

void print_text(uint8_t* text, uint8_t length){
    size_t i;
    for (i = 0; i < length; i++)
    {
        printf("%02x", text[i]);
    }
    printf("\n");
}

void pkcs7_pad(uint8_t *block, uint8_t block_length) {
  // do nothing at the moment - not sure if it is neccesary for our purpose
}

void xor_blocks(uint8_t *block_1_and_result, uint8_t *block_2, uint8_t block_size) {
  uint8_t i;
  for (i = 0; i < block_size; i++) {
    block_1_and_result[i] ^= block_2[i];
  }
}

void aes_encrypt_cbc(uint8_t *plaintext_and_result, uint8_t *iv, uint32_t length, uint8_t *key) {
  const uint8_t block_size = AES_128_BLOCK_SIZE;
  uint8_t i;

  AES_128.set_key(key);

  uint32_t blocks_length = length / block_size;
  const uint8_t remainder = length % block_size;
  if (remainder > 0) {
    // Pad last block
    blocks_length = blocks_length + 1;
    pkcs7_pad(plaintext_and_result + blocks_length * block_size, remainder);
  }

  xor_blocks(plaintext_and_result, iv, block_size);
  AES_128.encrypt(plaintext_and_result);

  for (i = 1; i < blocks_length; i++) {
    xor_blocks(plaintext_and_result + i * block_size, plaintext_and_result + (i - 1) * block_size, block_size);
    AES_128.encrypt(plaintext_and_result + i * block_size);
  }
  
}

/*---------------------------------------------------------------------------*/
PROCESS(comcrypt_process, "Comcrypt process");
AUTOSTART_PROCESSES(&comcrypt_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(comcrypt_process, ev, data)
{
  // 16 = 128 bit key
  // Key: 8a0439ed 5d393558 9b7c77c8 62a7e135
  uint8_t key[AES_128_KEY_LENGTH] = { 
    0x8a, 0x04, 0x39, 0xed,
    0x5d, 0x39, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0xa7, 0xe1, 0x35 
  };

  // Plaintext: 111439ed465635589b7c77c86266e1358a1439ed5d8835589b7c77c862a7e1358a9939ed5d3935589b7c77c862a7e135
  uint8_t plainText[PLAIN_TEXT_SIZE] = {
    0x11, 0x14, 0x39, 0xed,
    0x46, 0x56, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0x66, 0xe1, 0x35,
    0x8a, 0x14, 0x39, 0xed,
    0x5d, 0x88, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0xa7, 0xe1, 0x35,
    0x8a, 0x99, 0x39, 0xed,
    0x5d, 0x39, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0xa7, 0xe1, 0x35 
  };

  // IV: 52096ad53036a538bf40a39e81f3d7fb
  static unsigned char iv[16] = {
	  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb
  };

  PROCESS_BEGIN();

  print_text(plainText, PLAIN_TEXT_SIZE);

  // AES_128.set_key(key);
  // AES_128.encrypt(plainText);
  aes_encrypt_cbc(plainText, iv, PLAIN_TEXT_SIZE, key);

  print_text(plainText, PLAIN_TEXT_SIZE);
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
