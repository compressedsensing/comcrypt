#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define SIGNAL_LEN 512
#define BLOCK_LEN SIGNAL_LEN * 2
#define DEBUG 1

#ifndef LOG_MODULE
#define LOG_MODULE "Comcrypt"
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

// Utility macros
#define CEIL_DIVIDE(x,y) ((x+y-1)/y) /* x -Input, y - Divisor*/

#endif