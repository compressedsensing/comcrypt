#include "contiki.h"
#include "./encrypt.h"
#include "./compression.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include "./configuration.h"

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define SEND_INTERVAL (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;

// #define SIGNAL_LEN 128
// #define BLOCK_LEN SIGNAL_LEN * 4

static const huffman_codeword huffman_codebook[1 << HUFFMAN_RESOLUTION] = {
    {0b1, 1}, {0b0000, 4}, {0b00100, 5}, {0b001111, 6}, {0b001010, 6}, {0b0010111, 7}, {0b0001100, 7}, {0b000110101, 9}, {0b0001101001, 10}, {0b00011011, 8}, {0b0010110, 7}, {0b000111, 6}, {0b001110, 6}, {0b00010, 5}, {0b00110, 5}, {0b01, 2}};

static const huffman_codeword huffman_eof = {0b0001101000, 10};

// AES_128_KEY_LENGTH = 16 = 128 bit key
// Key: 8a0439ed 5d393558 9b7c77c8 62a7e135
static const uint8_t key[AES_128_KEY_LENGTH] = {
    0x8a, 0x04, 0x39, 0xed,
    0x5d, 0x39, 0x35, 0x58,
    0x9b, 0x7c, 0x77, 0xc8,
    0x62, 0xa7, 0xe1, 0x35};
// IV: 52096ad53036a538bf40a39e81f3d7fb
static unsigned char iv[16] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb};

// static const int16_t sensor_data[SIGNAL_LEN] = {1941, 1941, 1941, 1941, 1941, 1941, 1941, 1941, 1947, 1947, 1953, {1947}, {1947}, {.full = 1949}, {.full = 1959}, {.full = 1964}, {.full = 1968}, {.full = 1970}, {.full = 1972}, {.full = 1970}, {.full = 1961}, {.full = 1955}, {.full = 1957}, {.full = 1953}, {.full = 1947}, {.full = 1949}, {.full = 1941}, {.full = 1935}, {.full = 1937}, {.full = 1939}, {.full = 1935}, {.full = 1931}, {.full = 1925}, {.full = 1923}, {.full = 1916}, {.full = 1916}, {.full = 1910}, {.full = 1910}, {.full = 1910}, {.full = 1914}, {.full = 1923}, {.full = 1923}, {.full = 1925}, {.full = 1929}, {.full = 1931}, {.full = 1929}, {.full = 1925}, {.full = 1929}, {.full = 1931}, {.full = 1933}, {.full = 1931}, {.full = 1929}, {.full = 1933}, {.full = 1931}, {.full = 1939}, {.full = 1939}, {.full = 1939}, {.full = 1939}, {.full = 1939}, {.full = 1935}, {.full = 1933}, {.full = 1937}, {.full = 1939}, {.full = 1941}, {.full = 1945}, {.full = 1953}, {.full = 1966}, {.full = 1982}, {.full = 2017}, {.full = 2060}, {.full = 2119}, {.full = 2193}, {.full = 2277}, {.full = 2361}, {.full = 2441}, {.full = 2508}, {.full = 2537}, {.full = 2529}, {.full = 2459}, {.full = 2344}, {.full = 2181}, {.full = 1992}, {.full = 1796}, {.full = 1628}, {.full = 1513}, {.full = 1474}, {.full = 1509}, {.full = 1579}, {.full = 1667}, {.full = 1757}, {.full = 1822}, {.full = 1869}, {.full = 1904}, {.full = 1921}, {.full = 1927}, {.full = 1929}, {.full = 1925}, {.full = 1925}, {.full = 1921}, {.full = 1927}, {.full = 1933}, {.full = 1943}, {.full = 1945}, {.full = 1949}, {.full = 1949}, {.full = 1951}, {.full = 1949}, {.full = 1947}, {.full = 1947}, {.full = 1951}, {.full = 1953}, {.full = 1961}, {.full = 1972}, {.full = 1980}, {.full = 1982}, {.full = 1990}, {.full = 1984}, {.full = 1986}, {.full = 1982}, {.full = 1982}, {.full = 1980}, {.full = 1982}, {.full = 1982}, {.full = 1982}, {.full = 1986}, {.full = 1986}, {.full = 1984}};
static const int16_t sensor_data[SIGNAL_LEN] = { 242,242,242,242,242,242,242,242,243,243,244,243,243,243,244,245,246,246,
 246,246,245,244,244,244,243,243,242,241,242,242,241,241,240,240,239,239,
 238,238,238,239,240,240,240,241,241,241,240,241,241,241,241,241,241,241,
 242,242,242,242,242,241,241,242,242,242,243,244,245,247,252,257,264,274,
 284,295,305,313,317,316,307,293,272,249,224,203,189,184,188,197,208,219,
 227,233,238,240,240,241,240,240,240,240,241,242,243,243,243,243,243,243,
 243,243,244,245,246,247,247,248,248,248,247,247,247,247,247,247,248,248,
 248,247 };

static const int16_t threshhold = 0b0000001100000000;
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
  uint16_t i;
  for (i = 0; i < datalen; i++)
  {
    LOG_INFO_("%02x", data[i]);
  }
  LOG_INFO_("\n");
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(comcrypt_process, ev, data)
{
  PROCESS_BEGIN();
  static huffman_metadata h_data;
  int16_t signal[SIGNAL_LEN];
  uint8_t signal_bytes[BLOCK_LEN] = {0};
  uint16_t i;

  static struct etimer periodic_timer;
  uip_ipaddr_t dest_ipaddr;

  // Read signal
  for (i = 0; i < SIGNAL_LEN; i++) {
    signal[i] = sensor_data[i];
  }

  // Pipeline
  LOG_INFO_("Initial data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%08x", (unsigned int)signal[i]);
  }
  LOG_INFO_("\n");

  COMPRESS.dct_transform(signal, SIGNAL_LEN);

  LOG_INFO_("Transformed data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%08x", (unsigned int)signal[i]);
    // printf("%.2f\t", FP.fixed_to_float16(signal[i]));
  }
  LOG_INFO_("\n");

  COMPRESS.threshold(signal, threshhold, SIGNAL_LEN);

  LOG_INFO_("Thresholded data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%08x", (unsigned int)signal[i]);
  }
  LOG_INFO_("\n");

  // Fixed point to bytes
  for (i = 0; i < BLOCK_LEN; i += 4)
  {
    signal_bytes[i + 0] = (uint8_t)((signal[i >> 2] & 0xFF000000) >> 24);
    signal_bytes[i + 1] = (uint8_t)((signal[i >> 2] & 0x00FF0000) >> 16);
    signal_bytes[i + 2] = (uint8_t)((signal[i >> 2] & 0x0000FF00) >> 8);
    signal_bytes[i + 3] = (uint8_t)((signal[i >> 2] & 0x000000FF) >> 0);
  }

  LOG_INFO_("Byte data:\n");
  for (i = 0; i < BLOCK_LEN; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");

  h_data = COMPRESS.huffman_encode(signal_bytes, BLOCK_LEN, huffman_codebook, huffman_eof);

  LOG_INFO("Encoded data\n");
  for (i = 0; i < h_data.byte_length; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");

  ENCRYPT.aes_encrypt_ctr(signal_bytes, iv, h_data.byte_length, key);

  // LOG_INFO("Encrypted data\n");
  LOG_INFO_("Final data:\n");
  for (i = 0; i < h_data.byte_length; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
  etimer_set(&periodic_timer, SEND_INTERVAL);
  while (1)
  {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr))
    {
      /* Send to DAG root */
      simple_udp_sendto(&udp_conn, signal_bytes, h_data.byte_length, &dest_ipaddr);
    }
    else
    {
      LOG_INFO("Not reachable yet\n");
    }
    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND);
  }

  PROCESS_END();
  /*---------------------------------------------------------------------------*/
}