#pragma once

#include <stdint.h>

typedef struct {
  volatile uint32_t clk_div;
  volatile uint32_t exec_ctrl;
  volatile uint32_t shift_ctrl;
  volatile uint32_t addr;
  volatile uint32_t instr;
  volatile uint32_t pin_ctrl;
  volatile uint32_t tx_dma;
  volatile uint32_t rx_dma;
} pio_sm_t;

#define PIO_BASE 0xf0000000
#define PIO_CTRL   (PIO_BASE + 0)
#define PIO_FSTAT  (PIO_BASE + 4)
#define PIO_FDEBUG (PIO_BASE + 8)
#define PIO_FLEVEL (PIO_BASE + 0xc)

// bit4 of the FLEVEL values, joining tx+rx puts you over what the old FLEVEL can represent
#define PIO_FLEVEL_HI     (PIO_BASE + 0x010)

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
#define PIO_SM0_TX_DMA    (PIO_BASE + 0x0e4)
#define PIO_SM0_RX_DMA    (PIO_BASE + 0x0e8)

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

static volatile pio_sm_t *const pio_state_machines = (volatile pio_sm_t*) (PIO_BASE + 0xcc);
static volatile uint32_t *const pio_tx = (volatile uint32_t*) (PIO_BASE + 0x14);
static volatile uint32_t *const pio_rx = (volatile uint32_t*) (PIO_BASE + 0x24);
