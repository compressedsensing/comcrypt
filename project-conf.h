// #define UIP_CONF_TCP 0
// #define UIP_CONF_UDP 1

// #undef NETSTACK_CONF_WITH_IPV6
// #define NETSTACK_CONF_WITH_IPV6 0

// #undef UIP_CONF_IPV6_RPL
// #define UIP_CONF_IPV6_RPL 0
#undef NBR_TABLE_CONF_MAX_NEIGHBORS
#define NBR_TABLE_CONF_MAX_NEIGHBORS 4

#undef NETSTACK_MAX_ROUTE_ENTRIES
#define NETSTACK_MAX_ROUTE_ENTRIES 4

#undef UIP_CONF_BUFFER_SIZE
#define UIP_CONF_BUFFER_SIZE 240

#undef UIP_CONF_UDP_CONNS
#define UIP_CONF_UDP_CONNS 2

#ifndef FLOAT
#define FLOAT 1
#endif

// #undef QUEUEBUF_CONF_NUM
// #define QUEUEBUF_CONF_NUM 6

// #undef LOG_MODULE
// #define LOG_MODULE "App"

// #undef LOG_LEVEL
// #define LOG_LEVEL LOG_LEVEL_INFO
