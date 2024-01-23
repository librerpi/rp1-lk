#include <lk/console_cmd.h>
#include <lk/reg.h>
#include <stdio.h>

#define DMA_BASE 0x40188000

#define DMAC_CHEN 0x018

#define SAR       0x000
#define DAR       0x008
#define BLOCK_TS  0x010
#define CTL       0x018
#define CFG       0x020
#define STATUS    0x030
#define SSTAT     0x060
#define DSTAT     0x068

static int cmd_dma_dump(int argc, const console_cmd_args *argv) {
  uint32_t tl, th;
  if (argc > 1) {
    uint32_t chanbase = DMA_BASE + 0x100 + (argv[1].u * 0x100);
#define dumpreg(reg) th = *REG32(chanbase + reg + 4); tl = *REG32(chanbase + reg); printf(#reg":\t 0x%x_%08x\n", th, tl)
    dumpreg(SAR);
    dumpreg(DAR);
    dumpreg(BLOCK_TS);
    dumpreg(CTL);
    printf("  DST_MSIZE: %d\n", (tl >> 18) & 0xf);
    printf("  SRC_MSIZE: %d\n", (tl >> 14) & 0xf);
    printf("  DST_TR_WIDTH: %d\n", (tl >> 11) & 7);
    printf("  SRC_TR_WIDTH: %d\n", (tl >> 8) & 7);
    dumpreg(CFG);
    printf("  DST_PER: 0x%x\n", (tl >> 11) & 0x3f);
    printf("  SRC_PER: 0x%x\n", (tl >> 4) & 0x3f);
    dumpreg(STATUS);
    dumpreg(SSTAT);
    dumpreg(DSTAT);
#undef dumpreg
  } else {
#define dumpreg(reg) th = *REG32(DMA_BASE + reg + 4); tl = *REG32(DMA_BASE + reg); printf(#reg":\t 0x%x_%08x\n", th, tl)
    dumpreg(DMAC_CHEN);
  }
  return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("dma_dump", "dump dma state", &cmd_dma_dump)
STATIC_COMMAND_END(dma);
