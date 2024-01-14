#include <platform/time.h>
#include <stdbool.h>
#include <lk/reg.h>
#include <arch/arm/cm.h>

int platform_dgetc(char *c, bool wait) {
  return -1;
}

void platform_dputc(char c) {
  while (*REG32(0x40034000 + 0x18) & (1<<5));
  *REG32(0x40034000) = c; // uart1
}
void platform_early_init(void) {
    // start the systick timer
    arm_cm_systick_init(200 * 1000 * 1000);
}
