#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
	uint32_t oldvalue;
	uint32_t newvalue;
	char expression[50];
  struct watchpoint *next;

  /* TODO: Add more members if necessary */


} WP;

#endif
