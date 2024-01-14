#pragma once

typedef enum {
  DebugMonitor_IRQn = -1,
  SVCall_IRQn = -3,
  PendSV_IRQn = -2,
  SysTick_IRQn = -1
} IRQn_Type;

#define __NVIC_PRIO_BITS       2
