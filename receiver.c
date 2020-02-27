#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn;

#define SIGNAL_LEN 128
#define BLOCK_LEN SIGNAL_LEN * 4

/*---------------------------------------------------------------------------*/
PROCESS(receive_process, "Receive process");
AUTOSTART_PROCESSES(&receive_process);

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
  for (i = 0; i < datalen; i++) {
    LOG_INFO_("%02x", data[i]);
  }
  if (datalen < 128) {
    LOG_INFO_("\n\n");
  }
  // simple_udp_sendto(&udp_conn, data, datalen, sender_addr);
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(receive_process, ev, data)
{
  PROCESS_BEGIN();

  /* Initialize DAG root */
  NETSTACK_ROUTING.root_start();

  /* Initialize UDP connection */
  simple_udp_register(&udp_conn, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
  

  PROCESS_END();
  /*---------------------------------------------------------------------------*/
}