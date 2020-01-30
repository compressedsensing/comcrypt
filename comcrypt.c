#include "contiki.h"
#include <stdio.h>
#include "./encrypt.h"

#define PLAIN_TEXT_SIZE        AES_128_BLOCK_SIZE * 3

void print_text(uint8_t* text, uint8_t length){
    size_t i;
    for (i = 0; i < length; i++)
    {
        printf("%02x", text[i]);
    }
    printf("\n");
}



/*---------------------------------------------------------------------------*/
PROCESS(comcrypt_process, "Comcrypt process");
AUTOSTART_PROCESSES(&comcrypt_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(comcrypt_process, ev, data)
{
  // AES_128_KEY_LENGTH = 16 = 128 bit key
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

  ENCRYPT.aes_encrypt_cbc(plainText, iv, PLAIN_TEXT_SIZE, key);

  print_text(plainText, PLAIN_TEXT_SIZE);
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
