#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __linux__
#include <netinet/in.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

#include <stdio.h>
#include <time.h>

#include "typedef.h"

#define SMP_WMB()
#define SMP_RMB()
#define SMP_MB()

#define ATOMIC_EXEC(code)                                                                          \
  do {                                                                                             \
    volatile U32 _primask = __get_PRIMASK();                                                       \
    __disable_irq();                                                                               \
    code;                                                                                          \
    __set_PRIMASK(_primask);                                                                       \
  } while (0)

#define NANO_PER_SEC      (1000000000ULL) // 10^9
#define MICRO_PER_SEC     (1000000ULL)    // 10^6
#define MILLI_PER_SEC     (1000ULL)       // 10^3

#define WIN_TO_UNIX_EPOCH (116444736000000000ULL)

static inline U64
get_mono_timestamp_ns(void) {
#ifdef __linux__
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return ts.tv_sec * NANO_PER_SEC + ts.tv_nsec;
#elif defined(_WIN32)
  LARGE_INTEGER frequency, counter;
  QueryPerformanceFrequency(&frequency);
  QueryPerformanceCounter(&counter);
  return counter.QuadPart * 1000000000ull / frequency.QuadPart;
#endif
}

static inline U64
get_mono_timestamp_us(void) {
  return get_mono_timestamp_ns() / 1000ull;
}

static inline U64
get_mono_timestamp_ms(void) {
  return get_mono_timestamp_ns() / 1000000ull;
}

static inline U64
get_mono_timestamp_s(void) {
  return get_mono_timestamp_ns() / 1000000000ull;
}

static inline U64
get_real_timestamp_ns(void) {
#ifdef __linux__
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return ts.tv_sec * NANO_PER_SEC + ts.tv_nsec;
#elif defined(_WIN32)
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);

  ULARGE_INTEGER uli;
  uli.LowPart  = ft.dwLowDateTime;
  uli.HighPart = ft.dwHighDateTime;

  return (uli.QuadPart - WIN_TO_UNIX_EPOCH) * 100u;
#endif
}

static inline U64
get_real_timestamp_us(void) {
  return get_real_timestamp_ns() / 1000u;
}

static inline U64
get_real_timestamp_ms(void) {
  return get_real_timestamp_ns() / 1000000ull;
}

static inline U64
get_real_timestamp_s(void) {
  return get_real_timestamp_ns() / 1000000000ull;
}

static inline I32
format_timestamp_ms(U64 timestamp_ms, char *buf, U32 len) {
  time_t    seconds = timestamp_ms / 1000u;
  int       milli   = timestamp_ms % 1000u;
  struct tm tm_time;

#ifdef __linux__
  localtime_r(&seconds, &tm_time);
#elif defined(_WIN32)
  localtime_s(&tm_time, &seconds);
#else
  struct tm *tmp = localtime(&seconds);
  if (tmp)
    tm_time = *tmp;
#endif

  // "YYYY-MM-DDTHH:MM:SS.mmm"
  return snprintf(buf,
                  len,
                  "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
                  tm_time.tm_year + 1900,
                  tm_time.tm_mon + 1,
                  tm_time.tm_mday,
                  tm_time.tm_hour,
                  tm_time.tm_min,
                  tm_time.tm_sec,
                  milli);
}

static inline void
swap_byte_order(void *data, const U32 len) {
  U32 *p = (U32 *)data;
  for (U32 i = 0; i < len / 4; ++i) {
    // p[i] = ntohl(p[i]);
  }
}

#ifdef __cplusplus
}
#endif

#endif // !UTIL_H
