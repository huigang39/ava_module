#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "fifo/fifo.h"
#include "util/util.h"

#define LOGGER_TIMESTAMP_SIZE (32)
#define LOGGER_LEVEL_SIZE     (16)
#define LOGGER_MSG_SIZE       (256)
#define FIFO_BUF_SIZE         (1024 * 1024)

typedef enum {
  LOGGER_LEVEL_DATA,  // 数据
  LOGGER_LEVEL_DEBUG, // 调试
  LOGGER_LEVEL_INFO,  // 一般
  LOGGER_LEVEL_WARN,  // 警告
  LOGGER_LEVEL_ERROR, // 错误
} logger_level_e;

typedef enum {
  LOGGER_TEXT,
  LOGGER_BINARY,
} logger_type_e;

typedef void (*logger_print_f)(FILE *file, char *str, U32 len);
typedef U64 (*logger_ts_f)(void);

typedef struct {
  logger_ts_f    f_ts;
  logger_print_f f_print;
} logger_ops_t;

typedef struct {
  logger_level_e e_level;       // 最低级别
  logger_type_e  e_type;        // 过滤类型
  char           new_line_sign; // 换行符
  const char    *prefix;        // 前缀
} logger_cfg_t;

typedef struct {
  FILE *file;
} logger_in_t;

typedef struct {
  char ts[LOGGER_TIMESTAMP_SIZE];
  char level[LOGGER_LEVEL_SIZE];
  U32  len;
  U8   msg[LOGGER_MSG_SIZE];
} logger_out_t;

typedef struct {
  sfifo_t fifo;
} logger_lo_t;

typedef struct {
  logger_cfg_t cfg;
  logger_in_t  in;
  logger_out_t out;
  logger_lo_t  lo;
  logger_ops_t ops;
} logger_t;

#define DECL_LOGGER_PTRS(logger)                                                                   \
  logger_t     *p   = (logger);                                                                    \
  logger_cfg_t *cfg = &p->cfg;                                                                     \
  logger_in_t  *in  = &p->in;                                                                      \
  logger_out_t *out = &p->out;                                                                     \
  logger_lo_t  *lo  = &p->lo;                                                                      \
  logger_ops_t *ops = &p->ops;

#define DECL_LOGGER_PTRS_PREFIX(logger, prefix)                                                    \
  logger_t     *prefix##_p   = (logger);                                                           \
  logger_cfg_t *prefix##_cfg = &prefix##_p->cfg;                                                   \
  logger_in_t  *prefix##_in  = &prefix##_p->in;                                                    \
  logger_out_t *prefix##_out = &prefix##_p->out;                                                   \
  logger_lo_t  *prefix##_lo  = &prefix##_p->lo;                                                    \
  logger_ops_t *prefix##_ops = &prefix##_p->ops;

static inline void
logger_ts_to_str(U64 ts, char *buf) {
  snprintf(buf, LOGGER_TIMESTAMP_SIZE, "[%llu]", ts);
}

static inline void
logger_level_to_str(logger_level_e e_level, char *buf) {
  switch (e_level) {
  case LOGGER_LEVEL_DATA:
    strcpy(buf, "[DATA]");
    return;
  case LOGGER_LEVEL_DEBUG:
    strcpy(buf, "[DEBUG]");
    return;
  case LOGGER_LEVEL_INFO:
    strcpy(buf, "[INFO]");
    return;
  case LOGGER_LEVEL_WARN:
    strcpy(buf, "[WARN]");
    return;
  case LOGGER_LEVEL_ERROR:
    strcpy(buf, "[ERROR]");
    return;
  default:
    strcpy(buf, "[UNKNOW]");
    return;
  }
}

static inline void
logger_header_add(U64 ts, logger_level_e e_level, logger_out_t *out) {
  logger_ts_to_str(get_real_ts_ms(), out->ts);
  logger_level_to_str(e_level, out->level);
}

static inline void
logger_init(logger_t *logger, logger_cfg_t logger_cfg) {
  DECL_LOGGER_PTRS(logger);

  *cfg = logger_cfg;

  sfifo_t *fifo = &lo->fifo;
  SFIFO_INIT(fifo);
}

static inline void
logger_flush(logger_t *logger) {
  DECL_LOGGER_PTRS(logger);

  U8 str[sizeof(logger_out_t)];

  sfifo_t *fifo = &lo->fifo;
  FIFO_GET(fifo, str, sizeof(out));
  ops->f_print(in->file, (char *)str, sizeof(out));
}

static inline void
logger_write(logger_t *logger, const char *format, ...) {
  DECL_LOGGER_PTRS(logger);

  va_list args;
  va_start(args, format);
  out->len = vsnprintf((char *)out->msg, LOGGER_MSG_SIZE, format, args);
  va_end(args);

  sfifo_t *fifo = &lo->fifo;
  FIFO_PUT(fifo, out, sizeof(*out));
  logger_flush(logger);
}

#ifdef __cplusplus
}
#endif

#endif // !LOGGER_H
