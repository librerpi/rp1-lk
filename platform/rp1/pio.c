#include <app.h>
#include <kernel/thread.h>
#include <stdio.h>
#include <platform/gpio.h>
#include <lk/console_cmd.h>
#include <platform/pio.h>


#define PICO_NO_HARDWARE 1
#include "uart_tx.h"

static int watch_regs(void*);
thread_t *watcher_thread;

static void pio_entry(const struct app_descriptor *app, void *args) {
  int sm_idx = 0;
  watcher_thread = thread_create("reg watcher", watch_regs, NULL, DEFAULT_PRIORITY, ARCH_DEFAULT_STACK_SIZE);
  thread_resume(watcher_thread);

  *REG32(PIO_FDEBUG) = 0xf | (0xf << 8) | (0xf << 16) | (0xf << 24); // clear all FDEBUG warnings
  *REG32(PIO_CTRL) = 0; // stop all SM's

  volatile pio_sm_t *const sm = &pio_state_machines[sm_idx];

  rp1_gpio_set_ctrl(8, 7, 4);
  int outbase = 8;
  int outcount = 1;

  int setbase = 8;
  int setcount = 1;
  int sideset_count = 2;
  int sideset_base = 8;

  sm->pin_ctrl = (outbase & 0x1f) | ((setbase & 0x1f) << 5) | ((setcount & 7) << 26) | ((sideset_count & 0x7) << 29) | ((outcount & 0x1f) << 20) | ((sideset_base & 0x1f) << 10);

  int progbase = 0;
  for (unsigned int i=0; i<(sizeof(uart_tx_program_instructions) / sizeof(uart_tx_program_instructions[0])); i++) {
    printf("%d 0x%x -> %d\n", i, uart_tx_program_instructions[i], i+progbase);
    *REG32(PIO_INSTR_MEM0 + ((i + progbase) * 4)) = uart_tx_program_instructions[i];
  }
  int wrap_bottom = progbase + uart_tx_wrap_target;
  int wrap_top = progbase + uart_tx_wrap;
  sm->exec_ctrl = ((wrap_bottom & 0x1f) << 7) | ((wrap_top & 0x1f) << 12) | (1<<30);
  sm->clk_div = 217 << 16;

  sm->instr = 0xe081; // set output mode

  uint32_t mask = 1 << sm_idx;

  *REG32(PIO_CTRL) = mask | (mask<<4) | (mask<<8); // enable and reset state

  // allow dma to the tx fifo
  sm->tx_dma = 0x80000104;

  //while (true) {
    for (int i=32; i<127; i++) {
      pio_tx[sm_idx] = i;
      thread_sleep(100);
    }
    pio_tx[sm_idx] = '\n';
    //puts("loop");
  //}
}

static int watch_regs(void *arg) {
  uint32_t fstat = 0;
  uint32_t fdebug = 0;
  uint32_t flevel = 0;
  uint32_t unk1 = 0, unk2 = 0;
  uint32_t unk;
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

    t = *REG32(PIO_SM0_TX_DMA);
    if (t != unk1) printf("TX_DMA 0x%x -> 0x%x\n", unk1, t);
    unk1 = t;

    t = *REG32(PIO_SM0_RX_DMA);
    if (t != unk2) printf("RX_DMA 0x%x -> 0x%x\n", unk2, t);
    unk2 = t;

    t = *REG32(PIO_FLEVEL_HI);
    if (t != unk) printf("FLEVEL_HI 0x%x -> 0x%x\n", unk, t);
    unk = t;

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
