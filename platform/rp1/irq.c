#include <arch/arch_ops.h>
#include <dev/uart/pl011.h>
#include <lk/debug.h>

void uart0_IRQHandler(void) {
  arm_cm_irq_entry();
  bool resched = false;
  puts("uart0 irq");
  resched = pl011_uart_irq(0);
  arm_cm_irq_exit(resched);
}

void uart1_IRQHandler(void) {
  arm_cm_irq_entry();
  bool resched = false;
  resched = pl011_uart_irq(1);
  arm_cm_irq_exit(resched);
}
void uart2_IRQHandler(void) {
  arm_cm_irq_entry();
  bool resched = false;
  puts("uart2 irq");
  resched = pl011_uart_irq(2);
  arm_cm_irq_exit(resched);
}
void uart3_IRQHandler(void) {
  arm_cm_irq_entry();
  bool resched = false;
  puts("uart3 irq");
  resched = pl011_uart_irq(3);
  arm_cm_irq_exit(resched);
}
void uart4_IRQHandler(void) {
  arm_cm_irq_entry();
  bool resched = false;
  puts("uart4 irq");
  resched = pl011_uart_irq(4);
  arm_cm_irq_exit(resched);
}
void uart5_IRQHandler(void) {
  arm_cm_irq_entry();
  bool resched = false;
  puts("uart5 irq");
  resched = pl011_uart_irq(5);
  arm_cm_irq_exit(resched);
}
