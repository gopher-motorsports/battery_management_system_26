#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include <stdio.h>

#define DEBUG_ON 1
#if DEBUG_ON
	#define Debug(__info,...) printf("Debug: " __info,##__VA_ARGS__)
#else
	#define Debug(__info,...)
#endif

#endif
