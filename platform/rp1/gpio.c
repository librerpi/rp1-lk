#include <lk/console_cmd.h>
#include <platform/gpio.h>
#include <stdio.h>
#include <string.h>

static int cmd_gpio_dump_state(int argc, const console_cmd_args *argv);
static int cmd_gpio_set(int argc, const console_cmd_args *argv);
static int cmd_gpio_clear(int argc, const console_cmd_args *argv);
static int cmd_gpio_func(int argc, const console_cmd_args *argv);
static int cmd_rio_direction(int argc, const console_cmd_args *argv);

STATIC_COMMAND_START
STATIC_COMMAND("gpio_dump_state", "dump current gpio state", &cmd_gpio_dump_state)
STATIC_COMMAND("gpio_set", "set a gpio high", &cmd_gpio_set)
STATIC_COMMAND("gpio_clear", "clear a gpio low", &cmd_gpio_clear)
STATIC_COMMAND("gpio_function", "set gpio function", &cmd_gpio_func)
STATIC_COMMAND("rio_dir", "set RIO pin direction", &cmd_rio_direction)
STATIC_COMMAND_END(gpio);

//volatile struct gpiocfg *cfg = (volatile struct gpiocfg*)(0x400d0000);
static const char *levels[] = { "LOW", "HIGH" };

static int cmd_gpio_dump_state(int argc, const console_cmd_args *argv) {
  for (int pin=0; pin<28; pin++) {
    uint32_t status = cfg[pin].status;
    uint32_t ctrl = cfg[pin].ctrl;
    int func = ctrl & 0x1f;
    int level = (status & (1<<17)) != 0;
    printf("GPIO%d = %s, status=0x%x, ctrl=0x%x, func=%d\n", pin, levels[level], status, ctrl, func);
  }
  return 0;
}

static int cmd_gpio_set(int argc, const console_cmd_args *argv) {
  int bank = 0;
  int pin = 0;
  if (argc < 2) {
    printf("usage: %s <pin> <bank>\n", argv[0].str);
    return 0;
  }
  if (argc > 2) bank = argv[2].u;
  pin = argv[1].u;
  rp1_gpio_set(bank, pin);
  return 0;
}

static int cmd_gpio_clear(int argc, const console_cmd_args *argv) {
  int bank = 0;
  int pin = 0;
  if (argc < 2) {
    printf("usage: %s <pin> <bank>\n", argv[0].str);
    return 0;
  }
  if (argc > 2) bank = argv[2].u;
  pin = argv[1].u;
  rp1_gpio_clear(bank, pin);
  return 0;
}

static int cmd_gpio_func(int argc, const console_cmd_args *argv) {
  // TODO, other banks
  int pin = 0;
  if (argc < 3) {
    printf("usage: %s <pin> <function>\n", argv[0].str);
    return 0;
  }
  pin = argv[1].u;
  int func = argv[2].u;
  int debounce = 4;
  rp1_gpio_set_ctrl(pin, func, debounce);
  return 0;
}

static int cmd_rio_direction(int argc, const console_cmd_args *argv) {
  int bank = 0;
  int pin = 0;
  int out = 0;
  if (argc < 3) {
    printf("usage: %s <pin> <dir> <bank>\n", argv[0].str);
    return 0;
  }
  if (argv > 3) bank = argv[3].u;

  if (strcmp(argv[2].str, "out")) out = 1;
  else if (strcmp(argv[2].str, "in")) out = 0;
  else out = argv[2].u;

  pin = argv[1].u;
  rp1_rio_set_direction(bank, pin, out);
  return 0;
}
