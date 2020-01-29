#include "contiki.h"
#include <stdio.h>
#include "lib/aes-128.h"

#define PLAIN_TEXT_SIZE        AES_128_BLOCK_SIZE

void print_text(uint8_t* text, uint8_t length){
    size_t i;
    for (i = 0; i < length; i++)
    {
        printf("%x", text[i]);
    }
    printf("\n");
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

  uint8_t plainText[PLAIN_TEXT_SIZE] = {
    0x11, 0x04, 0x39, 0xed,
    0x46, 0x56, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0x66, 0xe1, 0x35
    // 0x8a, 0x04, 0x39, 0xed,
    // 0x5d, 0x88, 0x35, 0x58,
    // 0x9b, 0x7c, 0x77, 0xc8,
    // 0x62, 0xa7, 0xe1, 0x35,
    // 0x8a, 0x99, 0x39, 0xed,
    // 0x5d, 0x39, 0x35, 0x58,
    // 0x9b, 0x7c, 0x77, 0xc8,
    // 0x62, 0xa7, 0xe1, 0x35 
  };

  PROCESS_BEGIN();

  print_text(plainText, PLAIN_TEXT_SIZE);

  AES_128.set_key(key);
  AES_128.encrypt(plainText);

  print_text(plainText, PLAIN_TEXT_SIZE);
  

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
