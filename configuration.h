#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define SIGNAL_LEN 128
#define BLOCK_LEN SIGNAL_LEN * 2
#define DEBUG 0

#ifndef LOG_MODULE
#define LOG_MODULE "Comcrypt"
#define LOG_LEVEL LOG_LEVEL_NONE
#endif

#endif