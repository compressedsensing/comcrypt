#undef AES_128_CONF
#define AES_128_CONF  aes_128_driver  

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

#undef UIP_CONF_UDP
#define UIP_CONF_UDP 0

// -----------------------------------