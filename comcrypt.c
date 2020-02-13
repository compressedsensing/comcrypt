#include "contiki.h"
#include <stdio.h>
#include "./encrypt.h"
#include "./compression.h"

#define PLAIN_TEXT_SIZE AES_128_BLOCK_SIZE * 3
#define SIGNAL_LEN 32
#define BLOCK_LEN SIGNAL_LEN * 4

const huffman_codeword huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
    {0b1, 1}, {0b0000, 4}, {0b00100, 5}, {0b001111, 6},
    {0b001010, 6}, {0b0010111, 7}, {0b0001100, 7}, {0b000110101, 9},
    {0b0001101001, 10}, {0b00011011, 8}, {0b0010110, 7}, {0b000111, 6},
    {0b001110, 6}, {0b00010, 5}, {0b00110, 5}, {0b01, 2}
};

const huffman_codeword huffman_eof = {0b0001101000, 10};

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
  PROCESS_BEGIN();
  // AES_128_KEY_LENGTH = 16 = 128 bit key
  // Key: 8a0439ed 5d393558 9b7c77c8 62a7e135
  uint8_t key[AES_128_KEY_LENGTH] = {
      0x8a, 0x04, 0x39, 0xed,
      0x5d, 0x39, 0x35, 0x58,
      0x9b, 0x7c, 0x77, 0xc8,
      0x62, 0xa7, 0xe1, 0x35};

  // IV: 52096ad53036a538bf40a39e81f3d7fb
  static unsigned char iv[16] = {
      0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb};

  FIXED11_21 threshhold;
  threshhold.part.integer = 3;
  threshhold.part.fraction = 5;
  FIXED11_21 sensor_data[SIGNAL_LEN];
  FIXED11_21 dct_result[SIGNAL_LEN];
  huffman_data h_data;
  uint8_t signal_bytes[BLOCK_LEN];
  uint8_t *aes_result;

  // Generate sensor data - should be real ECG data
  uint16_t i;
  for (i = 0; i < SIGNAL_LEN; i++) {
    sensor_data[i].part.integer = i % 12;
    sensor_data[i].part.fraction = (i * 7) % 12;
  }

  int32_t print_var;

  // Pipeline
  printf("Initial data: \n");
  for (i = 0; i < SIGNAL_LEN; i++) {
    print_var = sensor_data[i].full;
    printf("%08x", print_var);
  }

  COMPRESS.dct_transform(sensor_data, dct_result, SIGNAL_LEN);
  COMPRESS.threshold(dct_result, threshhold, SIGNAL_LEN);
  printf("\n\nThresholded data: \n");
  for (i = 0; i < SIGNAL_LEN; i++) {
    print_var = dct_result[i].full;
    printf("%08x", print_var);
  }

  // Fixed point to bytes
  for (i = 0; i < BLOCK_LEN; i+=4) {
    signal_bytes[i + 0] = (uint8_t)((dct_result[i >> 2].full & 0xFF000000) >> 24);
    signal_bytes[i + 1] = (uint8_t)((dct_result[i >> 2].full & 0x00FF0000) >> 16);
    signal_bytes[i + 2] = (uint8_t)((dct_result[i >> 2].full & 0x0000FF00) >> 8);
    signal_bytes[i + 3] = (uint8_t)((dct_result[i >> 2].full & 0x000000FF) >> 0);
  }

  h_data = COMPRESS.huffman_encode(signal_bytes, BLOCK_LEN, huffman_codebook, huffman_eof);
  printf("\n\nHuff encoded data: \n");
  for (i = 0; i < h_data.byte_length; i++) {
    printf("%02x", h_data.bits[i]);
  }

  printf("\n\nEncrypted data: \n");
  aes_result = h_data.bits;
  ENCRYPT.aes_encrypt_ctr(aes_result, iv, h_data.byte_length, key);
  for (i = 0; i < h_data.byte_length; i++) {
    printf("%02x", aes_result[i]);
  }

  free(h_data.bits);

  PROCESS_END();
/*---------------------------------------------------------------------------*/
}