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

// static struct simple_udp_connection udp_conn;
// static struct ctimer timer;
static uint16_t i = 0;
static uint8_t state = 0;
static huffman_metadata h_data;
static uint8_t signal_bytes[BLOCK_LEN] = {0};
// fe80::212:7400:1a45:c958
// static uip_ipaddr_t dest_ipaddr = {{0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x12, 0x74, 0x00, 0x1a, 0x45, 0xc9, 0x58}};

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

static int16_t signal[SIGNAL_LEN] = { 1941,1941,1941,1941,1941,1941,1941,1941,1947,1947,1953,1947,1947,1949,
 1959,1964,1968,1970,1972,1970,1961,1955,1957,1953,1947,1949,1941,1935,
 1937,1939,1935,1931,1925,1923,1916,1916,1910,1910,1910,1914,1923,1923,
 1925,1929,1931,1929,1925,1929,1931,1933,1931,1929,1933,1931,1939,1939,
 1939,1939,1939,1935,1933,1937,1939,1941,1945,1953,1966,1982,2017,2060,
 2119,2193,2277,2361,2441,2508,2537,2529,2459,2344,2181,1992,1796,1628,
 1513,1474,1509,1579,1667,1757,1822,1869,1904,1921,1927,1929,1925,1925,
 1921,1927,1933,1943,1945,1949,1949,1951,1949,1947,1947,1951,1953,1961,
 1972,1980,1982,1990,1984,1986,1982,1982,1980,1982,1982,1982,1986,1986,
 1984,1982,1980,1980,1978,1978,1974,1978,1976,1982,1984,1986,1986,1986,
 1994,1994,1996,2004,2007,2011,2023,2031,2039,2048,2058,2068,2072,2072,
 2080,2086,2088,2095,2099,2103,2109,2117,2119,2121,2117,2113,2105,2107,
 2103,2107,2101,2095,2093,2086,2078,2066,2056,2045,2037,2021,2015,2004,
 1998,1992,1988,1982,1978,1972,1964,1961,1953,1945,1939,1937,1939,1935,
 1935,1937,1937,1937,1935,1933,1933,1929,1927,1923,1927,1929,1933,1937,
 1941,1941,1943,1937,1939,1935,1937,1937,1941,1947,1941,1945,1949,1947,
 1953,1947,1943,1943,1945,1943,1947,1947,1949,1947,1945,1945,1943,1943,
 1943,1937,1937,1935,1935,1935,1935,1935,1935,1937,1931,1927,1921,1921,
 1912,1910,1918,1921 };
static const int16_t threshhold = 0b0000000000000000;
// static const int16_t threshhold = FP.float_to_fixed16(0.8);
/*---------------------------------------------------------------------------*/
PROCESS(comcrypt_process, "Comcrypt process");
AUTOSTART_PROCESSES(&comcrypt_process);

// static void
// udp_rx_callback(struct simple_udp_connection *c,
//                 const uip_ipaddr_t *sender_addr,
//                 uint16_t sender_port,
//                 const uip_ipaddr_t *receiver_addr,
//                 uint16_t receiver_port,
//                 const uint8_t *data,
//                 uint16_t datalen)
// {
//   for (i = 0; i < datalen; i++)
//   {
//     LOG_INFO_("%02x", data[i]);
//   }
//   LOG_INFO_("\n");
// }

static void convert_to_bytes() {
  for (i = 0; i < BLOCK_LEN; i += 2)
  {
    signal_bytes[i + 0] = (uint8_t)((signal[i >> 1] & 0xFF00) >> 8);
    signal_bytes[i + 1] = (uint8_t)((signal[i >> 1] & 0x00FF) >> 0);
  }
}

// static void send_packets() {
//   NETSTACK_RADIO.on();
//   uint8_t buf[128] = {0};
//   #if DEBUG
//   LOG_INFO("Sending to receiver mote\n");
//   #endif
//   for (i = 0; i <= h_data.byte_length / 128; i++) {
//     memset(buf, 0, 128);
//     memcpy(buf, signal_bytes + (i * 128), i == h_data.byte_length / 128 ? h_data.byte_length % 128 : 128);
//     simple_udp_sendto(&udp_conn, buf, i == h_data.byte_length / 128 ? h_data.byte_length % 128 : 128, &dest_ipaddr);
//   }
//   NETSTACK_RADIO.off();
// }

static void
callback()
{
  switch (state)
  {
  case 0:
  {
    LOG_INFO("Case 0\n");
    COMPRESS.dct_64_256(signal, SIGNAL_LEN);
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
  default:
    break;
  }
  state++;
  // ctimer_reset(&timer);
  // send_packets();
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
    callback();
    callback();
    callback();
    callback();
  /* Initialize UDP connection */
  // simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
  //                     UDP_SERVER_PORT, udp_rx_callback);

  // ctimer_set(&timer, 5 * CLOCK_SECOND, callback, NULL);
  // send_packets();

  PROCESS_END();
  /*---------------------------------------------------------------------------*/
}

// PROCESS_THREAD(action_process, ev, data)
// {
//   PROCESS_BEGIN();

//   PROCESS_END();
//   /*---------------------------------------------------------------------------*/
// }