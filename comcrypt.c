#include "contiki.h"
#include "./encrypt.h"
#include "./compression.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/ctimer.h"
#include "sys/log.h"
#include "./configuration.h"

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define SEND_INTERVAL (5 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static struct ctimer timer;
static huffman_metadata h_data;
static uint8_t signal_bytes[BLOCK_LEN] = {0};
static uip_ipaddr_t dest_ipaddr;

static const huffman_codeword huffman_codebook[16] = {
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

static int16_t signal[SIGNAL_LEN] = { 242,242,242,242,242,242,242,242,243,243,244,243,243,243,244,245,246,246,
 246,246,245,244,244,244,243,243,242,241,242,242,241,241,240,240,239,239,
 238,238,238,239,240,240,240,241,241,241,240,241,241,241,241,241,241,241,
 242,242,242,242,242,241,241,242,242,242,243,244,245,247,252,257,264,274,
 284,295,305,313,317,316,307,293,272,249,224,203,189,184,188,197,208,219,
 227,233,238,240,240,241,240,240,240,240,241,242,243,243,243,243,243,243,
 243,243,244,245,246,247,247,248,248,248,247,247,247,247,247,247,248,248,
 248,247,247,247,247,247,246,247,247,247,248,248,248,248,249,249,249,250,
 250,251,252,253,254,256,257,258,259,259,260,260,261,261,262,262,263,264,
 264,265,264,264,263,263,262,263,262,261,261,260,259,258,257,255,254,252,
 251,250,249,249,248,247,247,246,245,245,244,243,242,242,242,241,241,242,
 242,242,241,241,241,241,240,240,240,241,241,242,242,242,242,242,242,241,
 242,242,242,243,242,243,243,243,244,243,242,242,243,242,243,243,243,243,
 243,243,242,242,242,242,242,241,241,241,241,241,241,242,241,240,240,240,
 239,238,239,240,240,240,240,240,240,239,239,238,238,238,238,239,239,238,
 239,239,238,238,239,239,239,239,240,240,241,242,242,243,243,243,244,244,
 243,243,243,242,241,241,242,241,241,240,240,239,238,237,237,237,237,237,
 236,236,234,234,233,233,234,233,235,235,236,237,236,236,236,235,235,234,
 234,234,235,235,235,236,236,236,236,236,236,236,235,235,236,237,239,242,
 247,253,261,272,282,294,304,313,319,319,313,300,280,255,229,203,185,175,
 175,184,196,209,221,231,236,240,241,241,241,240,240,239,240,242,242,243,
 244,244,245,245,246,247,247,248,249,250,251,252,253,254,254,254,255,254,
 255,254,255,255,255,256,256,256,256,256,256,256,256,257,258,258,260,260,
 261,262,262,263,263,264,264,266,267,268,269,269,270,270,270,270,270,270,
 270,271,271,270,270,271,271,271,271,270,270,269,268,268,267,266,266,264,
 264,263,261,259,258,256,254,253,251,251,250,250,249,248,247,246,244,243,
 243,242,242,242,242,242,242,242,242,241,240,241,240,240,241,240,240,241,
 242,241,241,240,241,240,240,241,241,241,242,242,242,243,243,243,242,242,
 242,242,242,243,243,243,243,243 };
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

static void
callback(void *ptr)
{
  LOG_INFO("Starting transmission loop\n");
  LOG_INFO("Timer expired, check if receiver is reachable\n");
  if (NETSTACK_ROUTING.node_is_reachable() && NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
    /* Send to DAG root */
    LOG_INFO("Sending to receiver mote\n");
    simple_udp_sendto(&udp_conn, signal_bytes, h_data.byte_length, &dest_ipaddr);
  } else {
    LOG_INFO("Not reachable yet\n");
  }
  ctimer_reset(&timer);
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(comcrypt_process, ev, data)
{
  PROCESS_BEGIN();
  static uint16_t i;

  // Pipeline
  LOG_INFO_("Initial data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%04x", signal[i]);
  }
  LOG_INFO_("\n");

  COMPRESS.dct_transform(signal, SIGNAL_LEN);

  LOG_INFO_("Transformed data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%04x", signal[i]);
  }
  LOG_INFO_("\n");

  COMPRESS.threshold(signal, threshhold, SIGNAL_LEN);

  LOG_INFO_("Thresholded data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%04x", signal[i]);
  }
  LOG_INFO_("\n");

  // Fixed point to bytes
  for (i = 0; i < BLOCK_LEN; i += 2)
  {
    signal_bytes[i + 0] = (uint8_t)((signal[i >> 1] & 0xFF00) >> 8);
    signal_bytes[i + 1] = (uint8_t)((signal[i >> 1] & 0x00FF) >> 0);
  }

  LOG_INFO_("Byte data:\n");
  for (i = 0; i < BLOCK_LEN; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");

  h_data = COMPRESS.huffman_encode(signal_bytes, BLOCK_LEN, huffman_codebook, huffman_eof);
  
  if (h_data.success == -1) {
    LOG_INFO("Huff code was bigger than original block - skipping encoding\n");
  }

  LOG_INFO_("Encoded data\n");
  for (i = 0; i < h_data.byte_length; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");

  ENCRYPT.aes_encrypt_ctr(signal_bytes, iv, h_data.byte_length, key);

  LOG_INFO_("Final data:\n");
  for (i = 0; i < h_data.byte_length; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");

  LOG_INFO("Initialize UDP\n");
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
  LOG_INFO("UDP initialized - setting timer\n");
  LOG_INFO("Timer set\n");
  ctimer_set(&timer, 5 * CLOCK_SECOND, callback, NULL);

  PROCESS_END();
  /*---------------------------------------------------------------------------*/
}