#include "contiki.h"
#include <stdio.h>
#include "./encrypt.h"
#include "./compression.h"

#define SIGNAL_LEN 64
#define BLOCK_LEN SIGNAL_LEN * 4

static const huffman_codeword huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
    {0b1, 1}, {0b0000, 4}, {0b00100, 5}, {0b001111, 6}, {0b001010, 6}, {0b0010111, 7}, {0b0001100, 7}, {0b000110101, 9}, {0b0001101001, 10}, {0b00011011, 8}, {0b0010110, 7}, {0b000111, 6}, {0b001110, 6}, {0b00010, 5}, {0b00110, 5}, {0b01, 2}};

static const huffman_codeword huffman_eof = {0b0001101000, 10};

// AES_128_KEY_LENGTH = 16 = 128 bit key
// Key: 8a0439ed 5d393558 9b7c77c8 62a7e135
static uint8_t key[AES_128_KEY_LENGTH] = {
    0x8a, 0x04, 0x39, 0xed,
    0x5d, 0x39, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0xa7, 0xe1, 0x35};
// IV: 52096ad53036a538bf40a39e81f3d7fb
static unsigned char iv[16] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb};

// void print_text(uint8_t *text, uint8_t length)
// {
//   size_t i;
//   for (i = 0; i < length; i++)
//   {
//     printf("%02x", text[i]);
//   }
//   printf("\n");
// }

static const FIXED11_21 sensor_data[SIGNAL_LEN] = {{.full = 1941}, {.full = 1941}, {.full = 1941}, {.full = 1941}, {.full = 1941}, {.full = 1941}, {.full = 1941}, {.full = 1941}, {.full = 1947}, {.full = 1947}, {.full = 1953}, {.full = 1947}, {.full = 1947}, {.full = 1949}, {.full = 1959}, {.full = 1964}, {.full = 1968}, {.full = 1970}, {.full = 1972}, {.full = 1970}, {.full = 1961}, {.full = 1955}, {.full = 1957}, {.full = 1953}, {.full = 1947}, {.full = 1949}, {.full = 1941}, {.full = 1935}, {.full = 1937}, {.full = 1939}, {.full = 1935}, {.full = 1931}, {.full = 1925}, {.full = 1923}, {.full = 1916}, {.full = 1916}, {.full = 1910}, {.full = 1910}, {.full = 1910}, {.full = 1914}, {.full = 1923}, {.full = 1923}, {.full = 1925}, {.full = 1929}, {.full = 1931}, {.full = 1929}, {.full = 1925}, {.full = 1929}, {.full = 1931}, {.full = 1933}, {.full = 1931}, {.full = 1929}, {.full = 1933}, {.full = 1931}, {.full = 1939}, {.full = 1939}, {.full = 1939}, {.full = 1939}, {.full = 1939}, {.full = 1935}, {.full = 1933}, {.full = 1937}, {.full = 1939}};
static const FIXED11_21 threshhold = {.full = 0b00000000000000000001100000000000};

/*---------------------------------------------------------------------------*/
PROCESS(comcrypt_process, "Comcrypt process");
AUTOSTART_PROCESSES(&comcrypt_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(comcrypt_process, ev, data)
{
  PROCESS_BEGIN();
  huffman_data h_data;
  FIXED11_21 dct_result[SIGNAL_LEN];
  uint8_t signal_bytes[BLOCK_LEN] = {0};
  uint8_t *aes_result;
  uint16_t i;
  unsigned int print_var;

  // Pipeline
  printf("Initial data: \n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    print_var = sensor_data[i].full;
    printf("%08x", print_var);
  }

  COMPRESS.dct_transform((FIXED11_21 *)sensor_data, dct_result, SIGNAL_LEN);
  printf("\n\nDCT data: \n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    print_var = dct_result[i].full;
    printf("%08x", print_var);
  }
  COMPRESS.threshold((FIXED11_21 *)dct_result, threshhold, SIGNAL_LEN);
  printf("\n\nThresholded data: \n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    print_var = dct_result[i].full;
    printf("%08x", print_var);
  }

  // Fixed point to bytes
  for (i = 0; i < BLOCK_LEN; i += 4)
  {
    signal_bytes[i + 0] = (uint8_t)((dct_result[i >> 2].full & 0xFF000000) >> 24);
    signal_bytes[i + 1] = (uint8_t)((dct_result[i >> 2].full & 0x00FF0000) >> 16);
    signal_bytes[i + 2] = (uint8_t)((dct_result[i >> 2].full & 0x0000FF00) >> 8);
    signal_bytes[i + 3] = (uint8_t)((dct_result[i >> 2].full & 0x000000FF) >> 0);
  }

  h_data = COMPRESS.huffman_encode(signal_bytes, BLOCK_LEN, huffman_codebook, huffman_eof);
  printf("\n\nHuff encoded data: \n");
  for (i = 0; i < h_data.byte_length; i++)
  {
    printf("%02x", h_data.bits[i]);
  }

  printf("\n\nEncrypted data: \n");
  aes_result = h_data.bits;
  ENCRYPT.aes_encrypt_ctr(aes_result, iv, h_data.byte_length, key);
  for (i = 0; i < h_data.byte_length; i++)
  {
    printf("%02x", aes_result[i]);
  }

  free(h_data.bits);

  PROCESS_END();
  /*---------------------------------------------------------------------------*/
}