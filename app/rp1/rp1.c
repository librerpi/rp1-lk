#include <app.h>
#include <stdio.h>
#include <kernel/thread.h>

static void rp1_entry(const struct app_descriptor *app, void *args) {
  int foo = 0;
  while (true) {
    thread_sleep(60 * 1000);
    printf("foo: %d\n", foo);
    foo++;
  }
}

APP_START(rp1)
  .entry = rp1_entry,
APP_END
