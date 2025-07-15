#ifndef FIFO_H
#define FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__linux) || defined(_WIN32)
#include <pthread.h>
#else
#define pthread_spinlock_t void
#define pthread_spin_init(lock, flag)
#define pthread_spin_lock(lock)
#define pthread_spin_unlock(lock)
#endif

#include "util/typedef.h"

typedef struct {
  U32                w;
  U32                r;
  U32                size;
  U32                mask;
  pthread_spinlock_t lock;
  void              *buf;
} fifo_t;

#ifndef FIFO_BUF_SIZE
#define FIFO_BUF_SIZE (1024 * 1024)
#endif
typedef struct {
  U32                w;
  U32                r;
  U32                size;
  U32                mask;
  pthread_spinlock_t lock;
  U8                 buf[FIFO_BUF_SIZE];
} sfifo_t;

#define IS_POWER_OF_2(n)    (n != 0) && ((n & (n - 1)) == 0)
#define ROUNDUP_POW_OF_2(n) ((n == 0) ? 1 : (1 << (sizeof(n) * 8 - __builtin_clz(n - 1))))

#define SFIFO_INIT(fifo)                                                                           \
  do {                                                                                             \
    fifo->w = fifo->r = 0;                                                                         \
    fifo->mask        = fifo->size - 1;                                                            \
    fifo->size        = sizeof(fifo->buf);                                                         \
    pthread_spin_init(&fifo->lock, PTHREAD_PROCESS_PRIVATE);                                       \
  } while (0)

#define FIFO_INIT(fifo, buf, size)                                                                 \
  do {                                                                                             \
    if (!IS_POWER_OF_2(size))                                                                      \
      size = ROUNDUP_POW_OF_2(size);                                                               \
    fifo->w = fifo->r = 0;                                                                         \
    fifo->mask        = fifo->size - 1;                                                            \
    fifo->size        = size;                                                                      \
    fifo->buf         = buf;                                                                       \
    pthread_spin_init(&fifo->lock, PTHREAD_PROCESS_PRIVATE);                                       \
  } while (0)

#define FIFO_PUT(fifo, buf, len)                                                                   \
  do {                                                                                             \
    len = MIN(len, fifo->size - fifo->w + fifo->r);                                                \
    /* 1. 确保在开始向 fifo 输入字节之前，对 fifo->out 索引进行采样 */                             \
    SMP_MB();                                                                                      \
    /* 2. 将从 fifo->w 开始的数据放到 buf 末端 */                                                  \
    U32 l = MIN(len, fifo->size - (fifo->w & fifo->mask));                                         \
    memcpy(fifo->buf + (fifo->w & fifo->mask), buf, l);                                            \
    /* 3. 将剩下的数据(如果有)放到 buf 的开头 */                                                   \
    memcpy(fifo->buf, buf + l, len - l);                                                           \
    /* 4. 确保在更新 fifo->w 索引之前将字节添加到 fifo */                                          \
    SMP_WMB();                                                                                     \
    fifo->w += len;                                                                                \
  } while (0)

#define FIFO_GET(fifo, buf, len)                                                                   \
  do {                                                                                             \
    len = MIN(len, fifo->w - fifo->r);                                                             \
    /* 1. 确保在开始从 fifo 中删除数据之前，对 fifo->in 索引进行采样 */                            \
    smp_rmb();                                                                                     \
    /* 2. 从 fifo->r 获取数据，直到 buf 结束 */                                                    \
    U32 l = MIN(len, fifo->size - (fifo->r & fifo->mask));                                         \
    memcpy(buf, fifo->buf + (fifo->r & fifo->mask), l);                                            \
    /* 3. 从 buf 的起始位置获取其余部分(如果有) */                                                 \
    memcpy(buf + l, fifo->buf, len - l);                                                           \
    /* 4. 确保在更新 fifo->r 索引之前，删除 fifo 中的数据 */                                       \
    SMP_MB();                                                                                      \
    fifo->r += len;                                                                                \
  } while (0)

#define FIFO_PEEK(fifo, buf, len)                                                                  \
  do {                                                                                             \
    len = MIN(len, fifo->w - fifo->r);                                                             \
    /* 1. 确保在开始从 fifo 中删除数据之前，对 fifo->in 索引进行采样 */                            \
    smp_rmb();                                                                                     \
    /* 2. 从 fifo->out 获取数据，直到 buf 结束 */                                                  \
    U32 l = MIN(len, fifo->size - (fifo->r & fifo->mask));                                         \
    memcpy(buf, fifo->buf + (fifo->r & fifo->mask), l);                                            \
    /* 3. 从 buf 的起始位置获取其余部分(如果有) */                                                 \
    memcpy(buf + l, fifo->buf, len - l);                                                           \
    /* 4. 确保在更新 fifo->out 索引之前，删除 fifo 中的数据 */                                     \
    SMP_MB();                                                                                      \
  } while (0)

#define FIFO_LEN(fifo, len)                                                                        \
  do {                                                                                             \
    len = fifo->w - fifo->r;                                                                       \
  } while (0)

#define FIFO_PUT_SPINLOCK(fifo, buf, len)                                                          \
  do {                                                                                             \
    pthread_spin_lock(&fifo->lock);                                                                \
    FIFO_PUT(fifo, buf, len);                                                                      \
    pthread_spin_unlock(&fifo->lock);                                                              \
  } while (0)

#define FIFO_GET_SPINLOCK(fifo, buf, len)                                                          \
  do {                                                                                             \
    pthread_spin_lock(&fifo->lock);                                                                \
    FIFO_GET(fifo, buf, len);                                                                      \
    /*                                                                                             \
     * fifo 为空时主动重置指针, 确保下次操作从缓冲区起始位置连续读写,   \
     * 避免不必要的内存回绕，从而提升性能和简化逻辑                          \
     */                                                                                            \
    if (fifo->w == fifo->r)                                                                        \
      fifo->w = fifo->r = 0;                                                                       \
    pthread_spin_unlock(&fifo->lock);                                                              \
  } while (0)

#define FIFO_PEEK_SPINLOCK(fifo, buf, len)                                                         \
  do {                                                                                             \
    pthread_spin_lock(&fifo->lock);                                                                \
    FIFO_PEEK(fifo, buf, len);                                                                     \
    /*                                                                                             \
     * fifo 为空时主动重置指针, 确保下次操作从缓冲区起始位置连续读写,   \
     * 避免不必要的内存回绕，从而提升性能和简化逻辑                          \
     */                                                                                            \
    if (fifo->w == fifo->r)                                                                        \
      fifo->w = fifo->r = 0;                                                                       \
    pthread_spin_unlock(&fifo->lock);                                                              \
  } while (0)

#define FIFO_LEN_SPINLOCK(fifo, len)                                                               \
  do {                                                                                             \
    pthread_spin_lock(&fifo->lock);                                                                \
    FIFO_LEN(fifo, len);                                                                           \
    pthread_spin_unlock(&fifo->lock);                                                              \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif // FIFO_H
