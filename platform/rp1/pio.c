#include <app.h>
#include <kernel/thread.h>
#include <stdio.h>
#include <platform/gpio.h>
#include <lk/console_cmd.h>

// 0xf0000010 TXF0
#define PIO_BASE 0xf0000000

#define PIO_CTRL   (PIO_BASE + 0)
#define PIO_FSTAT  (PIO_BASE + 4)
#define PIO_FDEBUG (PIO_BASE + 8)
#define PIO_FLEVEL (PIO_BASE + 0xc)

#define PIO_TXF0   (PIO_BASE + 0x14)

// 0xf0000048 DBG_CFGINFO
// 0xf000004c INSTR_MEM0 ?
// 0xf00000cc SM0_CLKDIV ?
// 0xf00000e0 SM0_PINCTRL

// 0xf00000d0 SM0_EXECCTRL ?
// 0xf00000d4 SM0_SHIFTCTRL ?
#define PIO_SM0_ADDR (PIO_BASE + 0xd8)
#define PIO_SM0_INSTR (PIO_BASE + 0xdc)

#define PICO_NO_HARDWARE 1
#include "uart_tx.h"

static void pio_entry(const struct app_descriptor *app, void *args) {
  *REG32(PIO_FDEBUG) = 0xf | (0xf << 8) | (0xf << 16) | (0xf << 24); // clear all FDEBUG warnings
  rp1_gpio_set_ctrl(8, 7, 4);
  int outbase = 8;
  int outcount = 1;

  int setbase = 8;
  int setcount = 1;
  int sideset_count = 2;
  int sideset_base = 8;

  *REG32(0xf00000e0) = (outbase & 0x1f) | ((setbase & 0x1f) << 5) | ((setcount & 7) << 26) | ((sideset_count & 0x7) << 29) | ((outcount & 0x1f) << 20) | ((sideset_base & 0x1f) << 10);
  int progbase = 0;
  for (int i=0; i<(sizeof(uart_tx_program_instructions) / sizeof(uart_tx_program_instructions[0])); i++) {
    printf("%d 0x%x -> %d\n", i, uart_tx_program_instructions[i], i+progbase);
    *REG32(0xf000004c + ((i + progbase) * 4)) = uart_tx_program_instructions[i];
  }
  int wrap_bottom = progbase + uart_tx_wrap_target;
  int wrap_top = progbase + uart_tx_wrap;
  *REG32(0xf00000d0) = ((wrap_bottom & 0x1f) << 7) | ((wrap_top & 0x1f) << 12) | (1<<30);
  *REG32(0xf00000cc) = 217 << 16;

  *REG32(PIO_SM0_INSTR) = 0xe081; // set output mode

  *REG32(0xf0000000) = 1 | (1<<4) | (1<<8); // enable and reset state

  while (true) {
    for (int i=32; i<127; i++) {
      *REG32(PIO_TXF0) = i;
      thread_sleep(100);
    }
    *REG32(PIO_TXF0) = '\n';
    puts("loop");
  }
}

APP_START(pio)
  .entry = pio_entry,
APP_END

static int cmd_pio_push(int argc, const console_cmd_args *argv) {
  if (argc < 2) {
    puts("usage: pio_push <words> <words>\n");
    return 0;
  }
  for (int i=1; i<argc; i++) {
    *REG32(PIO_TXF0) = argv[i].u;
  }
  return 0;
}

static int cmd_pio_dump(int argc, const console_cmd_args *argv) {
  printf("PIO_CTRL: 0x%x\n", *REG32(PIO_CTRL));
  printf("PIO_FSTAT: 0x%x\n", *REG32(PIO_FSTAT));
  printf("PIO_FDEBUG: 0x%x\n", *REG32(PIO_FDEBUG));
  printf("PIO_FLEVEL: 0x%x\n", *REG32(PIO_FLEVEL));
  printf("PIO_SM0_ADDR: %d\n", *REG32(PIO_SM0_ADDR));
  printf("PIO_SM0_INSTR: 0x%x\n", *REG32(PIO_SM0_INSTR));
  return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("pio_push", "push 1 or more words to SM0 TX fifo", &cmd_pio_push)
STATIC_COMMAND("pio_dump", "dump all pio regs", &cmd_pio_dump)
STATIC_COMMAND_END(pio);
