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

static int16_t signal[SIGNAL_LEN] = {  939, 939, 940, 942, 943, 942, 940, 942, 943, 944, 943, 942, 944, 943,
  947, 947, 947, 947, 947, 945, 944, 946, 947, 948, 950, 954, 960, 968,
  985,1006,1035,1071,1112,1153,1192,1225,1239,1235,1201,1145,1065, 973,
  877, 795, 739, 720, 737, 771, 814, 858, 890, 913, 930, 938, 941, 942,
  940, 940, 938, 941, 944, 949, 950, 952, 952, 953, 952, 951, 951, 953,
  954, 958, 963, 967, 968, 972, 969, 970, 968, 968, 967, 968, 968, 968,
  970, 970, 969, 968, 967, 967, 966, 966, 964, 966, 965, 968, 969, 970,
  970, 970, 974, 974, 975, 979, 980, 982, 988, 992, 996,1000,1005,1010,
 1012,1012,1016,1019,1020,1023,1025,1027,1030,1034,1035,1036,1034,1032,
 1028,1029,1027,1029,1026,1023,1022,1019,1015,1009,1004, 999, 995, 987,
  984, 979, 976, 973, 971, 968, 966, 963, 959, 958, 954, 950, 947, 946,
  947, 945, 945, 946, 946, 946, 945, 944, 944, 942, 941, 939, 941, 942,
  944, 946, 948, 948, 949, 946, 947, 945, 946, 946, 948, 951, 948, 950,
  952, 951, 954, 951, 949, 949, 950, 949, 951, 951, 952, 951, 950, 950,
  949, 949, 949, 946, 946, 945, 945, 945, 945, 945, 945, 946, 943, 941,
  938, 938, 934, 933, 937, 938, 938, 940, 941, 941, 939, 937, 934, 933,
  932, 933, 932, 934, 935, 933, 935, 934, 933, 931, 934, 934, 935, 934,
  938, 941, 943, 946, 947, 950, 950, 953, 954, 955, 952, 952, 950, 949,
  945, 945, 946, 944, 944, 940, 938, 935, 931, 928, 927, 926, 926, 926,
  925, 922, 917, 917, 914, 914, 915, 913, 919, 921, 923, 926, 925, 925,
  922, 920, 918, 917, 916, 917, 918, 920, 921, 924, 924, 925, 923, 923,
  922, 922, 921, 921, 924, 928, 934, 947, 965, 992,1020,1063,1105,1149,
 1191,1225,1247,1248,1224,1172,1095, 999, 896, 796, 723, 684, 687, 720,
  768, 820, 864, 904, 924, 941, 944, 945, 944, 941, 939, 936, 941, 946,
  948, 953, 954, 957, 958, 959, 961, 966, 966, 970, 976, 978, 983, 987,
  989, 993, 996, 996, 997, 996, 997, 996, 997, 998, 999,1001,1002,1002,
 1002,1002,1000,1000,1002,1004,1009,1011,1016,1019,1022,1026,1027,1029,
 1030,1033,1035,1040,1045,1048,1051,1054,1057,1056,1055,1056,1058,1058,
 1056,1059,1060,1058,1058,1061,1061,1060,1060,1056,1055,1052,1050,1048,
 1046,1041,1040,1033,1032,1030,1022,1014,1009,1001, 995, 989, 984, 983,
  980, 979, 976, 972, 967, 962, 957, 953, 951, 949, 948, 949, 948, 947,
  947, 946, 947, 943, 941, 942, 940, 941, 942, 940, 941, 944, 946, 944,
  944, 941, 942, 941, 941, 942, 943, 943, 946, 947, 949, 952, 952, 950,
  949, 949, 948, 947, 948, 950, 951, 953, 952, 952, 949, 948, 946, 943,
  941, 939, 940, 939, 941, 941, 939, 937, 936, 931, 931, 928, 931, 931,
  932, 929, 931, 931, 932, 931, 931, 930, 933, 931, 933, 934, 935, 939,
  946, 949, 955, 957, 958, 961, 962, 962 };
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

  COMPRESS.fct(signal);
  #if DEBUG
  LOG_INFO_("Transformed data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%04x", signal[i]);
  }
  LOG_INFO_("\n");
  #endif

  COMPRESS.threshold(signal, threshhold, SIGNAL_LEN);
  #if DEBUG
  LOG_INFO_("Thresholded data:\n");
  for (i = 0; i < SIGNAL_LEN; i++)
  {
    LOG_INFO_("%04x", signal[i]);
  }
  LOG_INFO_("\n");
  #endif

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