#ifndef FIFO_H
#define FIFO_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__linux) || defined(_WIN32)
#include <pthread.h>
#else
#define pthread_spinlock_t            U32
#define pthread_spin_init(lock, flag) ((void)0)
#define pthread_spin_lock(lock)       ((void)0)
#define pthread_spin_unlock(lock)     ((void)0)
#endif

#include <string.h>

#include "util/mathdef.h"
#include "util/typedef.h"

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

typedef struct {
  U32                w;
  U32                r;
  U32                size;
  U32                mask;
  pthread_spinlock_t lock;
  void              *buf;
} fifo_t;

#define IS_POWER_OF_2(n)    (n != 0) && ((n & (n - 1)) == 0)
#define ROUNDUP_POW_OF_2(n) ((n == 0) ? 1 : (1 << (sizeof(n) * 8 - __builtin_clz(n - 1))))

#define SFIFO_INIT(fifo)                                                                           \
  do {                                                                                             \
    (fifo)->w = (fifo)->r = 0;                                                                     \
    (fifo)->size          = sizeof((fifo)->buf);                                                   \
    (fifo)->mask          = (fifo)->size - 1;                                                      \
    pthread_spin_init(&(fifo)->lock, PTHREAD_PROCESS_PRIVATE);                                     \
  } while (0)

#define FIFO_INIT(fifo, init_buf, bufsize)                                                         \
  do {                                                                                             \
    if (!IS_POWER_OF_2(bufsize))                                                                   \
      bufsize = ROUNDUP_POW_OF_2(bufsize);                                                         \
    (fifo)->w = (fifo)->r = 0;                                                                     \
    (fifo)->size          = (bufsize);                                                             \
    (fifo)->mask          = (fifo)->size - 1;                                                      \
    (fifo)->buf           = (init_buf);                                                            \
    pthread_spin_init(&(fifo)->lock, PTHREAD_PROCESS_PRIVATE);                                     \
  } while (0)

#define FIFO_PUT(fifo, txbuf, len)                                                                 \
  do {                                                                                             \
    U32 size = MIN((len), (fifo)->size - (fifo)->w + (fifo)->r);                                   \
    SMP_MB();                                                                                      \
    U32 l = MIN(size, (fifo)->size - ((fifo)->w & (fifo)->mask));                                  \
    memcpy((U8 *)(fifo)->buf + ((fifo)->w & (fifo)->mask), (U8 *)(txbuf), l);                      \
    memcpy((U8 *)(fifo)->buf, (U8 *)(txbuf) + l, size - l);                                        \
    SMP_WMB();                                                                                     \
    (fifo)->w += size;                                                                             \
  } while (0)

#define FIFO_GET(fifo, rxbuf, len)                                                                 \
  do {                                                                                             \
    U32 size = MIN((len), (fifo)->w - (fifo)->r);                                                  \
    SMP_RMB();                                                                                     \
    U32 l = MIN(size, (fifo)->size - ((fifo)->r & (fifo)->mask));                                  \
    memcpy((U8 *)(rxbuf), (U8 *)(fifo)->buf + ((fifo)->r & (fifo)->mask), l);                      \
    memcpy((U8 *)(rxbuf) + l, (U8 *)(fifo)->buf, size - l);                                        \
    SMP_MB();                                                                                      \
    (fifo)->r += size;                                                                             \
  } while (0)

#define FIFO_PEEK(fifo, rxbuf, len)                                                                \
  do {                                                                                             \
    U32 size = MIN((len), (fifo)->w - (fifo)->r);                                                  \
    SMP_RMB();                                                                                     \
    U32 l = MIN(size, (fifo)->size - ((fifo)->r & (fifo)->mask));                                  \
    memcpy((U8 *)(rxbuf), (U8 *)(fifo)->buf + ((fifo)->r & (fifo)->mask), l);                      \
    memcpy((U8 *)(rxbuf) + l, (U8 *)(fifo)->buf, size - l);                                        \
    SMP_MB();                                                                                      \
  } while (0)

#define FIFO_LEN(fifo)                                                                             \
  do {                                                                                             \
    (fifo)->w - (fifo)->r;                                                                         \
  } while (0)

#define FIFO_PUT_SPINLOCK(fifo, txbuf, len)                                                        \
  do {                                                                                             \
    pthread_spin_lock(&(fifo)->lock);                                                              \
    FIFO_PUT(fifo, (txbuf), (len));                                                                \
    pthread_spin_unlock(&(fifo)->lock);                                                            \
  } while (0)

#define FIFO_GET_SPINLOCK(fifo, rxbuf, len)                                                        \
  do {                                                                                             \
    pthread_spin_lock(&(fifo)->lock);                                                              \
    FIFO_GET((fifo), (rxbuf), (len));                                                              \
    if ((fifo)->w == (fifo)->r)                                                                    \
      (fifo)->w = (fifo)->r = 0;                                                                   \
    pthread_spin_unlock(&(fifo)->lock);                                                            \
  } while (0)

#define FIFO_PEEK_SPINLOCK(fifo, rxbuf, len)                                                       \
  do {                                                                                             \
    pthread_spin_lock(&(fifo)->lock);                                                              \
    FIFO_PEEK((fifo), (rxbuf), (len));                                                             \
    if ((fifo)->w == (fifo)->r)                                                                    \
      (fifo)->w = (fifo)->r = 0;                                                                   \
    pthread_spin_unlock(&(fifo)->lock);                                                            \
  } while (0)

#define FIFO_LEN_SPINLOCK(fifo)                                                                    \
  do {                                                                                             \
    pthread_spin_lock(&(fifo)->lock);                                                              \
    U32 len = FIFO_LEN((fifo));                                                                    \
    pthread_spin_unlock(&(fifo)->lock);                                                            \
    len;                                                                                           \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif // !FIFO_H
