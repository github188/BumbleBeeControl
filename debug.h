#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

#define DBG_PRINTF(_x_)(printf("%s(%d)--:" _x_, __FILE__,__LINE__))

#define ASSERT(x) ((x)|| (DBG_PRINTF("assertion failed:\"" ## #x"\"\n")))
  
#endif
