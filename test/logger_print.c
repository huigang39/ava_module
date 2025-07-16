#include <stdio.h>

#include "logger/logger.h"

logger_t logger;

static inline void
print(FILE *file, char *str, U32 len) {
  printf(str);
}

int
main() {
  logger_cfg_t logger_cfg;
  logger_init(&logger, logger_cfg);

  logger.ops.f_print = print;
  logger.in.file     = stdout;

  U64 cnt = 0;
  while (1) {
    logger_write(&logger, "cnt: %llu\n", cnt++);
    usleep(1000);
  }

  return 0;
}
