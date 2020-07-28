#undef AES_128_CONF
#define AES_128_CONF  aes_128_driver  

#define QUEUEBUF_CONF_ENABLED 1

// #define CC2420_CONF_CHANNEL 26  // select 15.4 channel for transmission and listening
// #define CSMA_CONF_MAX_FRAME_RETRIES 0
#define WITH_PKT_STORAGE 35
// #define CC2420_CONF_AUTOCRC 0
// #define CC2420_CONF_CHECKSUM 0
// #define NONCORESEC_CONF_SEC_LVL 0
// #define CSMA_CONF_LLSEC_SECURITY_LEVEL 0
// #define CSMA_CONF_SEND_SOFT_ACK 0
// #define WITH_SECURITY 0
// #define LLSEC802154_CONF_ENABLED 0
// #define PACKETBUF_ATTR_SECURITY_LEVEL 0
// #define NETSTACK_CONF_MAC nullmac_driver
// #define NETSTACK_CONF_RDC nullrdc_noframer_driver
// #define NETSTACK_CONF_FRAMER framer_nullmac
//#define CC2420_CONF_AUTOACK 0


// With uIP stack ---------------------------------
// #undef NBR_TABLE_CONF_MAX_NEIGHBORS
// #define NBR_TABLE_CONF_MAX_NEIGHBORS 4

// #undef NETSTACK_MAX_ROUTE_ENTRIES
// #define NETSTACK_MAX_ROUTE_ENTRIES 4

// #undef UIP_CONF_BUFFER_SIZE
// #define UIP_CONF_BUFFER_SIZE 240

// #undef UIP_CONF_UDP_CONNS
// #define UIP_CONF_UDP_CONNS 4

// #undef QUEUEBUF_CONF_NUM
// #define QUEUEBUF_CONF_NUM 11

// -----------------------------------

// With nullnet ----------------------

// Set the CCA threshold so high that it wont affect measurements
#undef CC2420_CONF_CCA_THRESH
#define CC2420_CONF_CCA_THRESH 0

#undef QUEUEBUF_CONF_NUM
#define QUEUEBUF_CONF_NUM 23

#undef UIP_CONF_UDP
#define UIP_CONF_UDP 0

// -----------------------------------