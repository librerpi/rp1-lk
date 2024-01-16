#pragma once

#include <lk/reg.h>

#define RP1_RW_OFFSET                   0x0000
#define RP1_XOR_OFFSET                  0x1000
#define RP1_SET_OFFSET                  0x2000
#define RP1_CLR_OFFSET                  0x3000

struct gpiocfg {
  volatile uint32_t status;
  volatile uint32_t ctrl;
};

static volatile struct gpiocfg *const cfg = (volatile struct gpiocfg*)(0x400d0000);
static volatile uint32_t *const pads_bank0_gpio = (volatile uint32_t*)(0x400f0000 + 0x4);

static void rp1_gpio_set_ctrl(int pin, int function, int debounce) {
  // TODO, other banks
  cfg[pin].ctrl = (function & 0x1f) | ((debounce & 0x7f) << 5);
}

static void rp1_set_pad(int pin, int slewfast, int schmitt, int pulldown, int pullup, int drive, int inputenable, int outputdisable) {
  pads_bank0_gpio[pin] = (slewfast != 0) | ((schmitt != 0) << 1) | ((pulldown != 0) << 2) | ((pullup != 0) << 3) | ((drive & 0x3) << 4) | ((inputenable != 0) << 6) | ((outputdisable != 0) << 7);
}

static void rp1_gpio_set(int bank, int pin) {
  uint32_t addr;
  switch (bank) {
  case 0:
    addr = 0x400e0000;
    break;
  case 1:
    addr = 0x400e4000;
    break;
  case 2:
    addr = 0x400e8000;
    break;
  default:
    return;
  }
  *REG32(addr | 0x2000) = 1 << pin;
}

static void rp1_gpio_clear(int bank, int pin) {
  uint32_t addr;
  switch (bank) {
  case 0:
    addr = 0x400e0000;
    break;
  case 1:
    addr = 0x400e4000;
    break;
  case 2:
    addr = 0x400e8000;
    break;
  default:
    return;
  }
  *REG32(addr | 0x3000) = 1 << pin;
}

static void rp1_rio_set_direction(int bank, int pin, int out) {
  uint32_t addr = 0x400e0000 + (0x4000 * bank);
  addr += 4;
  if (out) addr |= RP1_SET_OFFSET;
  else addr |= RP1_CLR_OFFSET;

  *REG32(addr) = 1<<pin;
}
