#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifdef __cplusplus
extern "C" {
#endif

#define _GNU_SOURCE

#ifdef __linux__
#include <pthread.h>
#include <sched.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include "util/mathdef.h"
#include "util/typedef.h"

#ifndef SCHED_TASK_MAX
#define SCHED_TASK_MAX 255
#endif

typedef struct {
  FP32 freq_hz;
  U8   cpu_id;
} sched_cfg_t;

typedef void (*sched_cb_f)(void *arg);

typedef struct {
  U32        id;           // 任务ID
  U64        delay;        // 初始延时
  U64        freq_hz;      // 执行频率
  U64        exec_cnt_max; // 执行次数
  sched_cb_f f_cb;         // 回调函数
  void      *arg;          // 回调参数
} sched_task_t;

typedef struct {

} sched_out_t;

typedef enum {
  SCHED_STATE_READY,
  SCHED_STATE_RUNNING,
  SCHED_STATE_BLOCKED,
  SCHED_STATE_SUSPENDED,
} sched_state_e;

typedef struct {
  sched_state_e e_state;
  U64           exec_cnt;
  U64           elapsed;
  U64           create_ts;
  U64           next_run_ts;
} sched_stat_t;

typedef struct {
  sched_task_t task;
  sched_stat_t stat;
} sched_in_t;

typedef struct {
  sched_in_t tasks[SCHED_TASK_MAX];
  U32        num_tasks;
  U64        curr_ts;
} sched_lo_t;

typedef U64 (*sched_ts_f)(void);

typedef struct {
  sched_ts_f f_ts;
} sched_ops_t;

typedef struct {
  sched_cfg_t cfg;
  sched_in_t  in;
  sched_out_t out;
  sched_lo_t  lo;
  sched_ops_t ops;
} sched_t;

#define DECL_SCHED_PTRS(sched)                                                                     \
  sched_t     *p   = (sched);                                                                      \
  sched_cfg_t *cfg = &p->cfg;                                                                      \
  sched_in_t  *in  = &p->in;                                                                       \
  sched_out_t *out = &p->out;                                                                      \
  sched_lo_t  *lo  = &p->lo;                                                                       \
  sched_ops_t *ops = &p->ops;

#define DECL_SCHED_PTRS_PREFIX(sched, prefix)                                                      \
  sched_t     *prefix##_p   = (sched);                                                             \
  sched_cfg_t *prefix##_cfg = &prefix##_p->cfg;                                                    \
  sched_in_t  *prefix##_in  = &prefix##_p->in;                                                     \
  sched_out_t *prefix##_out = &prefix##_p->out;                                                    \
  sched_lo_t  *prefix##_lo  = &prefix##_p->lo;                                                     \
  sched_ops_t *prefix##_ops = &prefix##_p->ops;

#ifdef __linux__
static void *
sched_thread_fn(void *arg) {
  sched_t *t = (sched_t *)arg;
  while (1)
    t->func.f_run(t);
  return NULL;
}
#elif defined(_WIN32)
static DWORD WINAPI
sched_thread_fn(LPVOID arg) {
  sched_t *t = (sched_t *)arg;
  while (1)
    t->func.f_run(t);
  return 0;
}
#endif

#ifdef __linux__
static void
bind_thread_to_cpu(pthread_t thread_tid, int cpu_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(cpu_id, &cpuset);

  int ret = pthread_setaffinity_np(thread_tid, sizeof(cpu_set_t), &cpuset);
  if (ret)
    printf("[SCHED] Set thread affinity failed, errcode: %d\n", ret);
  printf("[SCHED] Bind thread to CPU %d success\n", cpu_id);
}
#elif defined(_WIN32)
static void
bind_thread_to_cpu(HANDLE thread_handle, int cpu_id) {
  DWORD_PTR mask = 1 << cpu_id;

  DWORD_PTR ret = SetThreadAffinityMask(thread_handle, mask);
  if (!ret)
    printf("[SCHED] Set thread affinity failed, errcode: %lu\n", GetLastError());
  printf("[SCHED] Bind thread to CPU %d success\n", cpu_id);
}
#endif

static inline void
sched_init(sched_t *sched, sched_cfg_t sched_cfg) {
  DECL_SCHED_PTRS(sched);
  *cfg = sched_cfg;

#ifdef __linux__
  pthread_t sched_tid;
  int       ret = pthread_create(&sched_tid, NULL, sched_thread_fn, self);
  if (ret != 0) {
    printf("[SCHED] Create thread failed, errcode: %d\n", ret);
    return;
  }
#elif defined(_WIN32)
  DWORD  thread_id;
  HANDLE sched_tid = CreateThread(NULL,            // 默认安全属性
                                  0,               // 默认堆栈大小
                                  sched_thread_fn, // 线程函数
                                  self,            // 传递给线程函数的参数
                                  0,               // 默认创建标志
                                  &thread_id       // 用于接收线程ID
  );
  if (sched_tid == NULL) {
    printf("[SCHED] Create thread failed, errcode: %lu\n", GetLastError());
    return;
  }
#endif

#if defined(__linux__) || defined(_WIN32)
  bind_thread_to_cpu(sched_tid, cfg.cpu_id);
#endif
}

static inline void
sched_add_task(sched_t *sched, sched_task_t sched_task) {
  DECL_SCHED_PTRS(sched);

  lo->tasks[sched_task.id].task           = sched_task;
  lo->tasks[sched_task.id].stat.e_state   = SCHED_STATE_READY;
  lo->tasks[sched_task.id].stat.create_ts = ops->f_ts();
}

static inline void
sched_run(sched_t *sched) {
  DECL_SCHED_PTRS(sched);

  for (U32 i = 0; i < SCHED_TASK_MAX; i++) {
    sched_task_t *task = &lo->tasks[i].task;
    sched_stat_t *stat = &lo->tasks[i].stat;

    if (!task->f_cb)
      continue;

    if (stat->e_state != SCHED_STATE_READY)
      continue;

    if (ops->f_ts() < stat->next_run_ts)
      continue;

    U64 begin_ts  = ops->f_ts();
    stat->e_state = SCHED_STATE_RUNNING;
    task->f_cb(task->arg);
    stat->exec_cnt++;
    stat->e_state = SCHED_STATE_READY;
    U64 end_ts    = ops->f_ts();

    stat->elapsed = end_ts - begin_ts;

    if (0 != task->exec_cnt_max && stat->exec_cnt >= task->exec_cnt_max) {
      stat->e_state = SCHED_STATE_SUSPENDED;
      return;
    }

    stat->next_run_ts = ops->f_ts() + U32_HZ_TO_US(task->freq_hz);
  }
}

#ifdef __cplusplus
}
#endif

#endif // !SCHEDULER_H
