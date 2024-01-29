#include <arch/arch_ops.h>
#include <arch/arm/cm.h>
#include <dev/uart/pl011.h>
#include <kernel/novm.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <lk/console_cmd.h>
#include <lk/debug.h>
#include <lk/err.h>
#include <lk/reg.h>
#include <lk/trace.h>
#include <platform.h>
#include <platform/gpio.h>
#include <platform/platform_cm.h>
#include <platform/time.h>
#include <stdbool.h>

#define LOCAL_TRACE 0

static int cmd_uart3(int argc, const console_cmd_args *argv) {
  pl011_uart_putc(3, 'A');
  return 0;
}

#define CLK_BASE 0x40018000
#define FC0_REF_KHZ                     (CLK_BASE + 0x0021c)
#define FC0_MIN_KHZ                     (CLK_BASE + 0x00220)
#define FC0_MAX_KHZ                     (CLK_BASE + 0x00224)
#define FC0_DELAY                       (CLK_BASE + 0x00228)
#define FC0_INTERVAL                    (CLK_BASE + 0x0022c)
#define FC0_SRC                         (CLK_BASE + 0x00230)
#define FC0_STATUS                      (CLK_BASE + 0x00234)
#define FC0_RESULT                      (CLK_BASE + 0x00238)

#define FC_SIZE                         0x20
#define FC_COUNT                        8

#define FC0_STATUS_DONE     (1<<4)
#define FC0_STATUS_RUNNING  (1<<8)

// based on drivers/clk/clk-rp1.c from linux
static int measure_clock(unsigned int index, unsigned int src) {
  int fc_offset = index * FC_SIZE;
  if (src == 0) {
    puts("invalid src");
    return 0;
  }
  if (index >= FC_COUNT) {
    puts("invalid index");
    return 0;
  }
  unsigned int timeout = current_time() + 1000;
  while (*REG32(FC0_STATUS + fc_offset) & FC0_STATUS_RUNNING) {
    if (current_time() > timeout) {
      puts("timeout waiting for fc to halt");
      return 0;
    }
  }
  *REG32(FC0_REF_KHZ + fc_offset) = 50000;
  *REG32(FC0_MIN_KHZ + fc_offset) = 0;
  *REG32(FC0_MAX_KHZ + fc_offset) = 0x1ffffff;
  *REG32(FC0_INTERVAL + fc_offset) = 8;
  *REG32(FC0_DELAY + fc_offset) = 7;
  *REG32(FC0_SRC + fc_offset) = src;
  timeout = current_time() + 1000;
  while (*REG32(FC0_STATUS + fc_offset) & FC0_STATUS_DONE) {
    if (current_time() > timeout) {
      puts("timeout waiting for fc to halt");
      return 0;
    }
  }
  int result = *REG32(FC0_RESULT + fc_offset);
  *REG32(FC0_SRC + fc_offset) = 0;
  return result * 3650;
}

static int cmd_measure_clock(int argc, const console_cmd_args *argv) {
  int index = 0;
  int src = 1;
  if (argv < 2) {
    puts("usage: measure_clock <index> <src>");
    return 0;
  }
  index = argv[1].u;
  src = argv[2].u;
  int clock = measure_clock(index, src);
  printf("FC_NUM(%d, %d) == %d Hz\n", index, src, clock);
  return 0;
}
static int cmd_measure_clocks(int argc, const console_cmd_args *argv) {
  for (int index = 0; index < FC_COUNT; index++) {
    for (int src = 1; src < 8; src++) {
      int clock = measure_clock(index, src);
      printf("FC_NUM(%d, %d) == %d Hz\n", index, src, clock);
    }
  }
  return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("uart3", "test uart3", &cmd_uart3)
STATIC_COMMAND("measure_clock", "measure an rp1 clock", &cmd_measure_clock)
STATIC_COMMAND("measure_clocks", "measure all rp1 clocks", &cmd_measure_clocks)
STATIC_COMMAND_END(platform);

/* un-overridden irq handler */
void rp1_dummy_irq(void) {
    arm_cm_irq_entry();
    printf("ACTIVE0: 0x%x\n", NVIC->IABR[0]);
    printf("ACTIVE1: 0x%x\n", NVIC->IABR[1]);
    panic("unhandled irq\n");
}

/* a list of default handlers that are simply aliases to the dummy handler */
#define RP1_IRQ(name,num) \
void name##_IRQHandler(void) __WEAK_ALIAS("rp1_dummy_irq");
#include <platform/irqinfo.h>
#undef RP1_IRQ

const void* const __SECTION(".text.boot.vectab2") vectab2[64] = {
#define RP1_IRQ(name,num) [name##_IRQn] = name##_IRQHandler,
#include <platform/irqinfo.h>
#undef RP1_IRQ
};

//static bool uart_enabled = false;

int platform_dgetc(char *c, bool wait) {
    //thread_t *t = get_current_thread();
    //dump_thread(t);
    int ret = uart_getc(1, wait);
    if (ret == -1) {
        LTRACEF("fail\n");
        return -1;
    }
    *c = ret;
    LTRACEF("ret %d\n", ret);
    return 0;
}

void platform_dputc(char c) {
  //if (!uart_enabled) return;
  pl011_uart_putc(1, c);
  return;
  #if 0

  if ((*REG32(0x40034000 + 0x30) & 1) == 0) return;

  while (*REG32(0x40034000 + 0x18) & (1<<5));
  *REG32(0x40034000) = c; // uart1
#endif
}

static void rp1_gpio_pad_config(int pin, uint32_t flag) {
  volatile uint32_t *pad = REG32(0x400f0004);
  pad[pin] = flag;
}

static void mux_uart1_gpio01(void) {
  // Select the UART on GPIO 0 and 1 and enable them
  rp1_gpio_set_ctrl(0, 2, 4);
  rp1_gpio_set_ctrl(1, 2, 4);

  *REG32(0x400f0004) = 0x50;  // gpio0, Output enable, 4mA, no pull
  *REG32(0x400f0008) = 0xca;  // gpio1, Input enable, pull up, schmitt
}

static void mux_uart3_gpio89(void) {
  rp1_gpio_set_ctrl(8, 2, 4); // tx
  rp1_gpio_set_ctrl(9, 2, 4); // rx

  rp1_gpio_pad_config(8, 0x50);
  rp1_gpio_pad_config(9, 0xca);
}

static void whack_reset(int bank, int bit) {
  // https://github.com/G33KatWork/RP1-Reverse-Engineering/blob/master/reversing/resets.py#L158-L227
  *REG32((0x40014000 + (bank*4)) | 0x2000 ) = 1 << bit;
  for (int i=0; i<6666666; i++) asm volatile ("nop");
  *REG32((0x40014000 + (bank*4)) | 0x3000 ) = 1 << bit;
  for (int i=0; i<6666666; i++) asm volatile ("nop");
}

void platform_early_init(void) {
    //novm_add_arena("mem", 0x10002000, 8 * 1024);
    // start the systick timer
    arm_cm_systick_init(200 * 1000 * 1000);

    for (int i=0; i<6; i++) {
      pl011_uart_register(i, 0x40030000 + (0x4000 * i));
    }

    //whack_reset(1, 27); // uart1
    //whack_reset(0, 27); // PIO

    //pl011_uart_init_early(0, 0x40030000);
    mux_uart1_gpio01();
    mux_uart3_gpio89();
    pl011_uart_init_early(1);
    //pl011_uart_init_early(2, 0x40038000);
    pl011_uart_init_early(3);
    //pl011_uart_init_early(4, 0x40040000);
    //pl011_uart_init_early(5, 0x40044000);
    pl011_set_baud(1, 115200);
    pl011_set_baud(3, 115200);
}

void platform_init(void) {
  pl011_uart_init(1, uart1_IRQn);
  //pl011_uart_init(3, uart3_IRQn);
  puts("platform init finished");
}

status_t unmask_interrupt(unsigned int vector) {
  int group = vector/32;
  int bit = vector%32;
  NVIC->ISER[group] = 1 << bit;

  return NO_ERROR;
}
