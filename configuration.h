#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#define SIGNAL_LEN 512
#define BLOCK_LEN SIGNAL_LEN * 2

#ifndef DEBUG
#define DEBUG 0
#endif

#ifndef LOG_MODULE
#define LOG_MODULE "Comcrypt"
#define LOG_LEVEL LOG_LEVEL_NONE
#endif

// Utility macros
#define CEIL_DIVIDE(x,y) ((x+y-1)/y) /* x -Input, y - Divisor*/

#endif