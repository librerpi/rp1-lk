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
#include <platform/platform_cm.h>
#include <platform/time.h>
#include <stdbool.h>

#define LOCAL_TRACE 0

static int cmd_uart3(int argc, const console_cmd_args *argv) {
  pl011_uart_putc(3, 'A');
  return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("uart3", "test uart3", &cmd_uart3)
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
  //pl011_uart_putc(1, c);

  while (*REG32(0x40034000 + 0x18) & (1<<5));
  *REG32(0x40034000) = c; // uart1
}

static void rp1_gpio_pad_config(int pin, uint32_t flag) {
  volatile uint32_t *pad = REG32(0x400f0004);
  pad[pin] = flag;
}

struct gpiocfg {
  volatile uint32_t status;
  volatile uint32_t ctrl;
};

static void rp1_gpio_ctrl_config(int pin, uint32_t flag) {
  volatile struct gpiocfg *cfg = (volatile struct gpiocfg*)(0x400d0000);
  cfg[pin].ctrl = flag;
}

static void mux_uart1_gpio01(void) {
  // Select the UART on GPIO 0 and 1 and enable them
  *REG32(0x400d0004) = 0x82;  // gpio0 Function 2 = UART1
  *REG32(0x400d000c) = 0x82;  // gpio1

  *REG32(0x400f0004) = 0x50;  // gpio0, Output enable, 4mA, no pull
  *REG32(0x400f0008) = 0xca;  // gpio1, Input enable, pull up, schmitt
}

static void mux_uart3_gpio89(void) {
  rp1_gpio_ctrl_config(8, 0x82);
  rp1_gpio_ctrl_config(9, 0x82);

  rp1_gpio_pad_config(8, 0x50);
  rp1_gpio_pad_config(9, 0xca);
}


void platform_early_init(void) {
    //novm_add_arena("mem", 0x10002000, 8 * 1024);
    // start the systick timer
    arm_cm_systick_init(200 * 1000 * 1000);
    //pl011_uart_init_early(0, 0x40030000);
    //pl011_uart_init_early(1, 0x40034000);
    //mux_uart1_gpio01();
    mux_uart3_gpio89();
    //pl011_uart_init_early(2, 0x40038000);
    pl011_uart_init_early(3, 0x4003c000);
    //pl011_uart_init_early(4, 0x40040000);
    //pl011_uart_init_early(5, 0x40044000);
}

void platform_init(void) {
  pl011_uart_init(1, uart1_IRQn, 0x40034000);
  pl011_uart_init(3, uart3_IRQn, 0x4003c000);
}

status_t unmask_interrupt(unsigned int vector) {
  int group = vector/32;
  int bit = vector%32;
  NVIC->ISER[group] = 1 << bit;
  printf("irq %d turned on\n", vector);

  return NO_ERROR;
}


void platform_halt(platform_halt_action suggested_action, platform_halt_reason reason) {
/*
  if (suggested_action == HALT_ACTION_HALT) {
    dprintf(ALWAYS, "returning to ROM, reason: %d\n", reason);
    //*REG32(0x4015400c) = 0;
    arch_disable_ints();
  }
*/
  dprintf(ALWAYS, "HALT: spinning forever... (reason = %d)\n", reason);
  arch_disable_ints();
  for (;;);
}
