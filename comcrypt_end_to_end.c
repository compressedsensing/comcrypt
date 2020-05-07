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
static uint16_t i = 0;
static huffman_metadata h_data;
static uint8_t signal_bytes[BLOCK_LEN] = {0};
// static uint8_t *signal_bytes;
// fe80::212:7400:1a45:c958
static uip_ipaddr_t dest_ipaddr = {{0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x12, 0x74, 0x00, 0x1a, 0x45, 0xc9, 0x58}};


const huffman_codeword huffman_codebook[16] = {
    {0b0, 1}, {0b10011, 5}, {0b1110001, 7}, {0b10001, 5}, {0b111010, 6}, {0b111011, 6}, {0b111001, 6}, {0b10000, 5}, {0b10100, 5}, {0b10111, 5}, 
    {0b1101, 4}, {0b1100, 4}, {0b10110, 5}, {0b10010, 5}, {0b10101, 5}, {0b1111, 4}};

const huffman_codeword huffman_eof = {0b1110000, 7};

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

static int16_t signal[SIGNAL_LEN] = {  995, 995, 995, 995, 995, 995, 995, 995,1000, 997, 995, 994, 992, 993,
  992, 989, 988, 987, 990, 993, 989, 988, 986, 988, 993, 997, 993, 986,
  983, 977, 979, 975, 974, 972, 969, 969, 969, 971, 973, 971, 969, 966,
  966, 966, 966, 967, 965, 963, 967, 969, 969, 968, 967, 963, 966, 964,
  968, 966, 964, 961, 960, 957, 952, 947, 947, 943, 933, 927, 927, 939,
  958, 980,1010,1048,1099,1148,1180,1192,1177,1128,1058, 991, 951, 937,
  939, 950, 958, 959, 957, 955, 958, 959, 961, 962, 960, 957, 956, 959,
  955, 957, 958, 957, 958, 959, 958, 958, 955, 953, 957, 959, 963, 960,
  960, 958, 956, 957, 956, 955, 953, 953, 956, 958, 958, 958, 956, 954,
  959, 959 };
  
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

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(comcrypt_process, ev, data)
{
  static uint16_t dwt_len;

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

  dwt_len = COMPRESS.dwt_transform(signal);
  #if DEBUG
  LOG_INFO_("Transformed data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%04x", signal[i]);
  }
  LOG_INFO_("\n");
  #endif

  // COMPRESS.threshold(signal, threshhold, SIGNAL_LEN);
  // #if DEBUG
  // LOG_INFO_("Thresholded data:\n");
  // for (i = 0; i < SIGNAL_LEN; i++)
  // {
  //   LOG_INFO_("%04x", signal[i]);
  // }
  // LOG_INFO_("\n");
  // #endif

  convert_to_bytes();
  #if DEBUG
  LOG_INFO_("Byte data\n");
  for (i = 0; i < BLOCK_LEN; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");
  #endif
  h_data = COMPRESS.huffman_encode(signal_bytes, 2*dwt_len, huffman_codebook, huffman_eof);

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

  // printf("\n\n%d\n\n",h_data.byte_length);

  // for (i = 0; i < 16; i++)
  // {
  //   printf("%02x\t",iv[i]);
  //   /* code */
  // }
  
  // LOG_INFO_("\n");

  //   for (i = 0; i < AES_128_KEY_LENGTH; i++)
  // {
  //   printf("%02x\t",key[i]);
  //   /* code */
  // }
  // LOG_INFO_("\n\n");


  ENCRYPT.aes_encrypt_ctr(signal_bytes, iv, h_data.byte_length, key);

  #if DEBUG
  LOG_INFO_("Final data:\n");
  for (i = 0; i < h_data.byte_length; i++)
  {
    LOG_INFO_("%02x", signal_bytes[i]);
  }
  LOG_INFO_("\n");
  #endif

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  send_packets();

  PROCESS_END();
}