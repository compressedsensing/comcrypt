#include "contiki.h"
#include "net/nullnet/nullnet.h"
#include <string.h>
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define SIGNAL_LEN 128
#define BLOCK_LEN SIGNAL_LEN * 2

PROCESS(receive_process, "Receive process");
AUTOSTART_PROCESSES(&receive_process);

void input_callback(const void *data, uint16_t len,
  const linkaddr_t *src, const linkaddr_t *dest)
{
  uint16_t i;
  uint8_t p[BLOCK_LEN];
  memcpy(p, data, len);
  for (i = 0; i < len; i++) {
    LOG_INFO_("%02x", p[i]);
  }
}

PROCESS_THREAD(receive_process, ev, data)
{
  PROCESS_BEGIN();

  nullnet_set_input_callback(input_callback);

  PROCESS_END();
}