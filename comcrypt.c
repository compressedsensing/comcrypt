#include "contiki.h"
#include "./encrypt.h"
#include "./compression.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/ctimer.h"
#include "sys/log.h"
#include "./configuration.h"
#include "./fixedpoint.h"

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define SEND_INTERVAL (5 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static struct ctimer timer;
static uint16_t i = 0;
static uint8_t state = 0;
static huffman_metadata h_data;
static uint8_t signal_bytes[BLOCK_LEN] = {0};
// static uint8_t *signal_bytes;
// fe80::212:7400:1a45:c958
static uip_ipaddr_t dest_ipaddr = {{0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x12, 0x74, 0x00, 0x1a, 0x45, 0xc9, 0x58}};

const huffman_codeword huffman_codebook[16] = {
    {0b1, 1}, {0b0000, 4}, {0b00100, 5}, {0b001111, 6}, {0b001010, 6}, {0b0010111, 7}, {0b0001100, 7}, {0b000110101, 9}, {0b0001101001, 10}, {0b00011011, 8}, {0b0010110, 7}, {0b000111, 6}, {0b001110, 6}, {0b00010, 5}, {0b00110, 5}, {0b01, 2}};

const huffman_codeword huffman_eof = {0b0001101000, 10};

// AES_128_KEY_LENGTH = 16 = 128 bit key
// Key: 8a0439ed 5d393558 9b7c77c8 62a7e135
const uint8_t key[AES_128_KEY_LENGTH] = {
    0x8a, 0x04, 0x39, 0xed,
    0x5d, 0x39, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0xa7, 0xe1, 0x35};
// IV: 52096ad53036a538bf40a39e81f3d7fb
static uint8_t iv[16] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb};

static int16_t signal[SIGNAL_LEN] = { 469,469,470,471,471,471,470,471,471,472,471,471,472,471,473,473,473,473,
 473,472,472,473,473,474,475,477,480,484,492,503,517,535,556,576,596,612,
 619,617,600,572,532,486,438,397,369,360,368,385,407,429,445,456,465,469,
 470,471,470,470,469,470,472,474,475,476,476,476,476,475,475,476,477,479,
 481,483,484,486,484,485,484,484,483,484,484,484,485,485,484,484,483,483,
 483,483,482,483,482,484,484,485,485,485,487,487,487,489,490,491,494,496,
 498,500,502,505,506,506,508,509,510,511,512,513,515,517,517,518,517,516,
 514,514,513,514,513,511,511,509,507,504,502,499,497,493,492,489,488,486,
 485,484,483,481,479,479,477,475,473,473,473,472,472,473,473,473,472,472,
 472,471,470,469,470,471,472,473,474,474,474,473,473,472,473,473,474,475,
 474,475,476,475,477,475,474,474,475,474,475,475,476,475,475,475,474,474,
 474,473,473,472,472,472,472,472,472,473,471,470,469,469,467,466,468,469,
 469,470,470,470,469,468,467,466,466,466,466,467,467,466,467,467,466,465,
 467,467,467,467,469,470,471,473,473,475,475,476,477,477,476,476,475,474,
 472,472,473,472 };
static const int16_t threshhold = 0b0000000000000000;
/*---------------------------------------------------------------------------*/
PROCESS(comcrypt_process, "Comcrypt process");
AUTOSTART_PROCESSES(&comcrypt_process);

static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  for (i = 0; i < datalen; i++)
  {
    LOG_INFO_("%02x", data[i]);
  }
  LOG_INFO_("\n");
}

static void convert_to_bytes() {
  // uint16_t tmp = 0;
  // signal_bytes = (uint8_t*)signal;
  // for (i = 0; i < BLOCK_LEN; i += 2) {
  //   tmp = (uint8_t)((signal[i >> 1] & 0xFF00) >> 8);
  //   signal[i >> 1] <<= 8;
  //   signal[i >> 1] |= tmp;
  // }
  for (i = 0; i < BLOCK_LEN; i += 2)
  {
    signal_bytes[i + 0] = (uint8_t)((signal[i >> 1] & 0xFF00) >> 8);
    signal_bytes[i + 1] = (uint8_t)((signal[i >> 1] & 0x00FF) >> 0);
  }
}

static void send_packets() {
  NETSTACK_RADIO.on();
  uint8_t buf[128] = {0};
  #if DEBUG
  LOG_INFO("Sending to receiver mote\n");
  #endif
  for (i = 0; i <= h_data.byte_length / 128; i++) {
    memset(buf, 0, 128);
    memcpy(buf, signal_bytes + (i * 128), i == h_data.byte_length / 128 ? h_data.byte_length % 128 : 128);
    simple_udp_sendto(&udp_conn, buf, i == h_data.byte_length / 128 ? h_data.byte_length % 128 : 128, &dest_ipaddr);
  }
  NETSTACK_RADIO.off();
}

static void
callback(void *prt)
{
  switch (state)
  {
  case 0:
  {
    COMPRESS.dct_100_256(signal);
    #if DEBUG
    LOG_INFO_("Transformed data:\n");
    for (i = 0; i < SIGNAL_LEN; i++)
    {
      LOG_INFO_("%04x", signal[i]);
    }
    LOG_INFO_("\n");
    #endif
    break;
  }
  case 1:
  {
    COMPRESS.threshold(signal, threshhold, SIGNAL_LEN);
    #if DEBUG
    LOG_INFO_("Thresholded data:\n");
    for (i = 0; i < SIGNAL_LEN; i++)
    {
      LOG_INFO_("%04x", signal[i]);
    }
    LOG_INFO_("\n");
    #endif
    break;
  }
  case 2:
  {
    convert_to_bytes();
    #if DEBUG
    LOG_INFO_("Byte data\n");
    for (i = 0; i < BLOCK_LEN; i++)
    {
      LOG_INFO_("%02x", signal_bytes[i]);
    }
    LOG_INFO_("\n");
    #endif
    h_data = COMPRESS.huffman_encode(signal_bytes, BLOCK_LEN, huffman_codebook, huffman_eof);
  
    if (h_data.success == -1) {
      LOG_INFO("Huff code was bigger than original block - skipping encoding\n");
    }

    #if DEBUG
    LOG_INFO_("Encoded data\n");
    for (i = 0; i < h_data.byte_length; i++)
    {
      LOG_INFO_("%02x", signal_bytes[i]);
    }
    LOG_INFO_("\n");
    #endif
    break;
  }
  case 3:
  {
    ENCRYPT.aes_encrypt_ctr(signal_bytes, iv, h_data.byte_length, key);

    #if DEBUG
    LOG_INFO_("Final data:\n");
    for (i = 0; i < h_data.byte_length; i++)
    {
      LOG_INFO_("%02x", signal_bytes[i]);
    }
    LOG_INFO_("\n");
    #endif
    break;
  }
  case 4:
    send_packets();
    break;
  default:
    break;
  }
  state++;
  ctimer_reset(&timer);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(comcrypt_process, ev, data)
{
  PROCESS_BEGIN();
  NETSTACK_RADIO.off();
  #if DEBUG
    LOG_INFO_("Initial data:\n");
    for (i = 0; i < SIGNAL_LEN; i++)
    {
      LOG_INFO_("%04x", signal[i]);
    }
    LOG_INFO_("\n");
    #endif
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  ctimer_set(&timer, 5 * CLOCK_SECOND, callback, NULL);

  PROCESS_END();
}