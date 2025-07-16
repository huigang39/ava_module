#ifndef THREAD_H
#define THREAD_H

#ifdef __cpluscplus
extern "C" {
#endif

#include <stdio.h>

#define __USE_GNU
#ifdef __linux__
#include <pthread.h>
#include <sched.h>
static void *sched_thread_fn(void *arg);
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
#include <windows.h>
static DWORD WINAPI sched_thread_fn(LPVOID arg);
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
thread_init(void *arg, int cpu_id) {
#ifdef __linux__
  pthread_t sched_tid;
  int       ret = pthread_create(&sched_tid, NULL, sched_thread_fn, arg);
  if (ret != 0) {
    printf("[SCHED] Create thread failed, errcode: %d\n", ret);
    return;
  }
#elif defined(_WIN32)
  DWORD  thread_id;
  HANDLE sched_tid = CreateThread(NULL,            // 默认安全属性
                                  0,               // 默认堆栈大小
                                  sched_thread_fn, // 线程函数
                                  arg,             // 传递给线程函数的参数
                                  0,               // 默认创建标志
                                  &thread_id       // 用于接收线程ID
  );
  if (sched_tid == NULL) {
    printf("[SCHED] Create thread failed, errcode: %lu\n", GetLastError());
    return;
  }
#endif

#if defined(__linux__) || defined(_WIN32)
  bind_thread_to_cpu(sched_tid, cpu_id);
#endif
}

#ifdef __cpluscplus
}
#endif

#endif // !THREAD_H
