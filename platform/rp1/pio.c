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
#define PIO_TXF1   (PIO_BASE + 0x18)
#define PIO_TXF2   (PIO_BASE + 0x1c)
#define PIO_TXF3   (PIO_BASE + 0x20)

#define PIO_RXF0          (PIO_BASE + 0x24)
#define PIO_RXF1          (PIO_BASE + 0x28)
#define PIO_RXF2          (PIO_BASE + 0x2c)
#define PIO_RXF3          (PIO_BASE + 0x30)

#define PIO_DBG_PADOUT    (PIO_BASE + 0x040)
#define PIO_DBG_PADOE     (PIO_BASE + 0x044)

#define PIO_DBG_CFGINFO   (PIO_BASE + 0x48)
#define PIO_INSTR_MEM0    (PIO_BASE + 0x4c)

#define PIO_SM0_CLKDIV    (PIO_BASE + 0xcc)
#define PIO_SM0_EXECCTRL  (PIO_BASE + 0xd0)
#define PIO_SM0_SHIFTCTRL (PIO_BASE + 0xd4)
#define PIO_SM0_ADDR      (PIO_BASE + 0xd8)
#define PIO_SM0_INSTR     (PIO_BASE + 0xdc)
#define PIO_SM0_PINCTRL   (PIO_BASE + 0xe0)
#define PIO_SM0_UNK1      (PIO_BASE + 0x0e4)
#define PIO_SM0_UNK2      (PIO_BASE + 0x0e8)

#define PIO_SM1_CLKDIV    (PIO_BASE + 0x0ec)
#define PIO_SM1_EXECCTRL  (PIO_BASE + 0x0f0)
#define PIO_SM1_SHIFTCTRL (PIO_BASE + 0x0f4)
#define PIO_SM1_ADDR      (PIO_BASE + 0x0f8)
#define PIO_SM1_INSTR     (PIO_BASE + 0x0fc)
#define PIO_SM1_PINCTRL   (PIO_BASE + 0x100)

#define PIO_SM2_CLKDIV    (PIO_BASE + 0x10c)

#define PIO_SM3_CLKDIV    (PIO_BASE + 0x12c)
#define PIO_SM3_EXECCTRL  (PIO_BASE + 0x130)
#define PIO_SM3_SHIFTCTRL (PIO_BASE + 0x134)
#define PIO_SM3_ADDR      (PIO_BASE + 0x138)
#define PIO_SM3_INSTR     (PIO_BASE + 0x13c)
#define PIO_SM3_PINCTRL   (PIO_BASE + 0x140)


#define PICO_NO_HARDWARE 1
#include "uart_tx.h"

static int watch_regs(void*);
thread_t *watcher_thread;

static void pio_entry(const struct app_descriptor *app, void *args) {
  watcher_thread = thread_create("reg watcher", watch_regs, NULL, DEFAULT_PRIORITY, ARCH_DEFAULT_STACK_SIZE);
  thread_resume(watcher_thread);

  *REG32(PIO_FDEBUG) = 0xf | (0xf << 8) | (0xf << 16) | (0xf << 24); // clear all FDEBUG warnings
  *REG32(PIO_CTRL) = 0; // stop all SM's

  rp1_gpio_set_ctrl(8, 7, 4);
  int outbase = 8;
  int outcount = 1;

  int setbase = 8;
  int setcount = 1;
  int sideset_count = 2;
  int sideset_base = 8;

  *REG32(PIO_SM0_PINCTRL) = (outbase & 0x1f) | ((setbase & 0x1f) << 5) | ((setcount & 7) << 26) | ((sideset_count & 0x7) << 29) | ((outcount & 0x1f) << 20) | ((sideset_base & 0x1f) << 10);
  int progbase = 0;
  for (unsigned int i=0; i<(sizeof(uart_tx_program_instructions) / sizeof(uart_tx_program_instructions[0])); i++) {
    printf("%d 0x%x -> %d\n", i, uart_tx_program_instructions[i], i+progbase);
    *REG32(PIO_INSTR_MEM0 + ((i + progbase) * 4)) = uart_tx_program_instructions[i];
  }
  int wrap_bottom = progbase + uart_tx_wrap_target;
  int wrap_top = progbase + uart_tx_wrap;
  *REG32(PIO_SM0_EXECCTRL) = ((wrap_bottom & 0x1f) << 7) | ((wrap_top & 0x1f) << 12) | (1<<30);
  *REG32(PIO_SM0_CLKDIV) = 217 << 16;

  *REG32(PIO_SM0_INSTR) = 0xe081; // set output mode

  *REG32(PIO_CTRL) = 1 | (1<<4) | (1<<8); // enable and reset state

  //while (true) {
    for (int i=32; i<127; i++) {
      *REG32(PIO_TXF0) = i;
      thread_sleep(100);
    }
    *REG32(PIO_TXF0) = '\n';
    //puts("loop");
  //}
}

static int watch_regs(void *arg) {
  uint32_t fstat = 0;
  uint32_t fdebug = 0;
  uint32_t flevel = 0;
  uint32_t unk1 = 0, unk2 = 0;
  while (true) {
    uint32_t t = *REG32(PIO_FSTAT);
    if (t != fstat) printf("FSTAT 0x%x -> 0x%x\n", fstat, t);
    fstat = t;

    t = *REG32(PIO_FDEBUG);
    if (t != fdebug) printf("FDEBUG 0x%x -> 0x%x\n", fdebug, t);
    fdebug = t;
    *REG32(PIO_FDEBUG) = t;

    t = *REG32(PIO_FLEVEL);
    if (t != flevel) printf("FLEVEL 0x%x -> 0x%x\n", flevel, t);
    flevel = t;

    t = *REG32(PIO_SM0_UNK1);
    if (t != unk1) printf("UNK1 0x%x -> 0x%x\n", unk1, t);
    unk1 = t;

    t = *REG32(PIO_SM0_UNK2);
    if (t != unk2) printf("UNK2 0x%x -> 0x%x\n", unk2, t);
    unk2 = t;

    //thread_sleep(1);
    thread_yield();
  }
  return 0;
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
  printf("PIO_DBG_PADOUT: 0x%x\n", *REG32(PIO_DBG_PADOUT));
  printf("PIO_DBG_PADOE: 0x%x\n", *REG32(PIO_DBG_PADOE));

  printf("PIO_DBG_CFGINFO: 0x%x\n", *REG32(PIO_DBG_CFGINFO));

  printf("PIO_SM0_ADDR: %d\n", *REG32(PIO_SM0_ADDR));
  printf("PIO_SM0_INSTR: 0x%x\n", *REG32(PIO_SM0_INSTR));

  printf("PIO_SM1_ADDR: %d\n", *REG32(PIO_SM1_ADDR));
  printf("PIO_SM1_INSTR: 0x%x\n", *REG32(PIO_SM1_INSTR));
  return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("pio_push", "push 1 or more words to SM0 TX fifo", &cmd_pio_push)
STATIC_COMMAND("pio_dump", "dump all pio regs", &cmd_pio_dump)
STATIC_COMMAND_END(pio);
