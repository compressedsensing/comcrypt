#include "contiki.h"
#include <stdio.h>
#include "./encrypt.h"
#include "./compression.h"

#define PLAIN_TEXT_SIZE AES_128_BLOCK_SIZE * 3

const uint16_t huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
  0x0A98, 0x0152, 0x015E, 0x00A8, 
  0x0055, 0x0028, 0x000B, 0x0000, 
  0x0003, 0x0004, 0x0029, 0x0056, 
  0x00AE, 0x015F, 0x02A7, 0x054D, 
};

const uint16_t huffman_eof = 0xA99;

void print_text(uint8_t *text, uint8_t length)
{
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
  // // AES_128_KEY_LENGTH = 16 = 128 bit key
  // // Key: 8a0439ed 5d393558 9b7c77c8 62a7e135
  // AES_128_KEY_LENGTH = 16 = 128 bit key
  // Key: 8a0439ed 5d393558 9b7c77c8 62a7e135
  // uint8_t key[AES_128_KEY_LENGTH] = {
  //     0x8a, 0x04, 0x39, 0xed,
  //     0x5d, 0x39, 0x35, 0x58,
  //     0x9b, 0x7c, 0x77, 0xc8,
  //     0x62, 0xa7, 0xe1, 0x35};

  // // Plaintext: 111439ed465635589b7c77c86266e1358a1439ed5d8835589b7c77c862a7e1358a9939ed5d3935589b7c77c862a7e135
  // uint8_t plainText[PLAIN_TEXT_SIZE] = {
  //     0x62, 0x66, 0xe1, 0x35,
  //     0x8a, 0x14, 0x39, 0xed,
  //     0x5d, 0x88, 0x35, 0x58,
  //     0x9b, 0x7c, 0x77, 0xc8,
  //     0x62, 0xa7, 0xe1, 0x35,
  //     0x8a, 0x99, 0x39, 0xed,
  //     0x5d, 0x39, 0x35, 0x58,
  //     0x9b, 0x7c, 0x77, 0xc8,
  //     0x62, 0xa7, 0xe1, 0x35};

  float sensor_reading[12] = {5.4, 8.2, 3.7, 9.5, 3.7, 7.3, 1.2, 2.4, 6.6, 4.3, 7.2, 4.1};
  FIXED11_21 fixed_sensor_reading[12];

  float result[12] = {};
  FIXED11_21 fixed_result[12];
  unsigned int block_len = 12;

  // // IV: 52096ad53036a538bf40a39e81f3d7fb
  // Plaintext: 111439ed465635589b7c77c86266e1358a1439ed5d8835589b7c77c862a7e1358a9939ed5d3935589b7c77c862a7e135
  uint8_t plainText[PLAIN_TEXT_SIZE] = {
      0x66, 0x56, 0x35, 0x58,
      0x45, 0x77, 0x66, 0x35,
      0x77, 0x66, 0x66, 0x66,
      0x77, 0x77, 0x77, 0x66,
      0x77, 0x77, 0x77, 0x66,
      0x77, 0x66, 0x39, 0x66,
      0x66, 0x77, 0x66, 0x66,
      0x76, 0x77, 0x77, 0x66,
      0x66, 0x77, 0x67, 0x92};

  // float sensor_reading[12] = {5.4, 8.2, 3.7, 9.5, 3.7, 7.3, 1.2, 2.4, 6.6, 4.3, 7.2, 4.1};

  // float result[12] = {};
  // unsigned int block_len = 12;

  // IV: 52096ad53036a538bf40a39e81f3d7fb
  // static unsigned char iv[16] = {
  //     0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb};
  PROCESS_BEGIN();
  // printf("hithere");
  FIXED11_21 a, b, c;
  // a = FP.float_to_fixed(-5.5);
  // c = FP.fp_pow(a,3);
  // c = FP.fp_sin(a,8);

  FP.fp_add(a, b);
  // c = FP.fp_sin(a, 5);

  // c = a;
  // ENCRYPT.aes_encrypt_ctr(plainText, iv, PLAIN_TEXT_SIZE, key);

  // print_text(plainText, PLAIN_TEXT_SIZE);

  // c.full = a.full % b.full;
  // c.full = FP_PI21_16;

  // int32_t d = FP.factorial(5);


  // ENCRYPT.aes_encrypt_ctr(plainText, iv, PLAIN_TEXT_SIZE, key);

  // print_text(plainText, PLAIN_TEXT_SIZE);

  // printf("\n");

  int i;
  for (i = 0; i < block_len; i++)
  {
    fixed_sensor_reading[i] = FP.float_to_fixed(sensor_reading[i]);
  }

  b = FP.float_to_fixed(3.5);

  COMPRESS.dct_transform(&fixed_sensor_reading, &fixed_result, block_len);
  COMPRESS.threshold(&fixed_result, b, block_len);
  
  FIXED11_21 trunched[5];
  COMPRESS.simple_truncate(&fixed_result,&trunched,12);


  uint8_t *array;
  array = (uint8_t *)(&result);

  for (i = 0; i < 5; i++)
  {
    // printf("%.4f\t", FP.fixed_to_float(fixed_result[i]));
     printf("%.4f\t", FP.fixed_to_float(trunched[i]));
  }
  // ENCRYPT.aes_encrypt_ctr(array, iv, PLAIN_TEXT_SIZE, key);

  // printf("\n");

  // print_text(array, PLAIN_TEXT_SIZE);

  // printf("\n");
  // COMPRESS.dct_transform(&sensor_reading, &result, block_len);

  // array = (uint8_t *)(&result);
  // int i;
  // {
  //   printf("%.4f\t", result[i]);
  // }
  // ENCRYPT.aes_encrypt_ctr(array, iv, PLAIN_TEXT_SIZE, key);

  // printf("\n");

  // print_text(array, PLAIN_TEXT_SIZE);

  // printf("\n");
  struct huffman_data h;
  uint8_t remainder = 0;

  h = COMPRESS.huffman_encode(plainText, PLAIN_TEXT_SIZE, huffman_codebook, huffman_eof);

  printf("success: %d", h.success);

  printf("\n");

  if (h.length % 8) {
    remainder = 1;
  }
  print_text(h.bits, ((h.length >> 3) + remainder));
  printf("%d\n", h.length);
  printf("\n");
  printf("%d\n", h.length >> 3);
  printf("%d\n", PLAIN_TEXT_SIZE);

  free(h.bits);

  PROCESS_END();
/*---------------------------------------------------------------------------*/
}