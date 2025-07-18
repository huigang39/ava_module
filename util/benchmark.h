#ifndef BENCHMARK_H
#define BENCHMARK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM_MATH
#include "arm_math.h"
#endif

#include "fastmath.h"
#include "typedef.h"

#define DWT_INIT()                                                                                 \
  do {                                                                                             \
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;                                                \
    DWT->CYCCNT = (U32)0U;                                                                         \
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;                                                           \
  } while (0)

#define MEASURE_TIME(total, name, iterations, code)                                                \
  do {                                                                                             \
    volatile U32 _count = (iterations);                                                            \
    volatile U32 _start, _end;                                                                     \
    DWT->CYCCNT = 0;                                                                               \
    _start      = DWT->CYCCNT;                                                                     \
    for (volatile U32 i = 0; i < _count; i++) {                                                    \
      code;                                                                                        \
    }                                                                                              \
    _end = DWT->CYCCNT;                                                                            \
    if (_end < _start)                                                                             \
      total = 0xFFFFFFFF - _start + _end;                                                          \
    total = _end - _start;                                                                         \
  } while (0)

typedef struct {
  const char *name;
  U32         cycles_total;
  U32         cycles_per_op;
  FP32        ret;
} benchmark_t;

/* 整数运算 */                                                                                     \
#define TEST_INT_ADD(result, iterations)                                                           \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "int_add", iterations, { volatile int r = int_a + int_b; }); \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "int_add";                                                              \
    cnt++;                                                                                         \
  } while (0)

#define TEST_INT_MUL(result, iterations)                                                           \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "int_mul", iterations, { volatile int r = int_a * int_b; }); \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "int_mul";                                                              \
    cnt++;                                                                                         \
  } while (0)

#define TEST_INT_DIV(result, iterations)                                                           \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "int_div", iterations, { volatile int r = int_a / int_b; }); \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "int_div";                                                              \
    cnt++;                                                                                         \
  } while (0)

/* 浮点运算 */                                                                                     \
#define TEST_FLOAT_ADD(result, iterations)                                                         \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "float_add", iterations, { r = float_a + float_b; });        \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "float_add";                                                            \
    cnt++;                                                                                         \
  } while (0)

#define TEST_FLOAT_MUL(result, iterations)                                                         \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "float_mul", iterations, { r = float_a * float_b; });        \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "float_mul";                                                            \
    cnt++;                                                                                         \
  } while (0)

#define TEST_FLOAT_DIV(result, iterations)                                                         \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "float_div", iterations, { r = float_a / float_b; });        \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "float_div";                                                            \
    cnt++;                                                                                         \
  } while (0)

/* 三角函数 */                                                                                     \
#define TEST_SINF(result, iterations)                                                              \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "sinf", iterations, { r = sinf(angle); });                   \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "sinf";                                                                 \
    cnt++;                                                                                         \
  } while (0)

#define TEST_COSF(result, iterations)                                                              \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "cosf", iterations, { r = cosf(angle); });                   \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "cosf";                                                                 \
    cnt++;                                                                                         \
  } while (0)

#define TEST_TANF(result, iterations)                                                              \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "tanf", iterations, { r = tanf(angle); });                   \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "tanf";                                                                 \
    cnt++;                                                                                         \
  } while (0)

#ifdef ARM_MATH
#define TEST_ARM_SINF(result, iterations)                                                          \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "arm_sinf", iterations, { r = arm_sin_f32(angle); });        \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "arm_sinf";                                                             \
    cnt++;                                                                                         \
  } while (0)

#define TEST_ARM_COSF(result, iterations)                                                          \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "arm_cosf", iterations, { r = arm_cos_f32(angle); });        \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "arm_cosf";                                                             \
    cnt++;                                                                                         \
  } while (0)
#endif

#define TEST_FAST_SINF(result, iterations)                                                         \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "fast_sinf", iterations, { r = fast_sinf(angle); });         \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "fast_sinf";                                                            \
    cnt++;                                                                                         \
  } while (0)

#define TEST_FAST_COSF(result, iterations)                                                         \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "fast_cosf", iterations, { r = fast_cosf(angle); });         \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "fast_cosf";                                                            \
    cnt++;                                                                                         \
  } while (0)

#define TEST_FAST_TANF(result, iterations)                                                         \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "fast_tanf", iterations, { r = fast_tanf(angle); });         \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "fast_tanf";                                                            \
    cnt++;                                                                                         \
  } while (0)

#define TEST_ATAN2F_QUAD1(result, iterations)                                                      \
  do {                                                                                             \
    MEASURE_TIME(                                                                                  \
        result.cycles_total, "atan2f_quad1", iterations, { r = atan2f(y_val1, x_val1); });         \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "atan2f_quad1";                                                         \
    cnt++;                                                                                         \
  } while (0)

#define TEST_ATAN2F_QUAD2(result, iterations)                                                      \
  do {                                                                                             \
    MEASURE_TIME(                                                                                  \
        result.cycles_total, "atan2f_quad2", iterations, { r = atan2f(y_val2, x_val2); });         \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "atan2f_quad2";                                                         \
    cnt++;                                                                                         \
  } while (0)

#define TEST_ATAN2F_QUAD3(result, iterations)                                                      \
  do {                                                                                             \
    MEASURE_TIME(                                                                                  \
        result.cycles_total, "atan2f_quad3", iterations, { r = atan2f(y_val3, x_val3); });         \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "atan2f_quad3";                                                         \
    cnt++;                                                                                         \
  } while (0)

#define TEST_ATAN2F_QUAD4(result, iterations)                                                      \
  do {                                                                                             \
    MEASURE_TIME(                                                                                  \
        result.cycles_total, "atan2f_quad4", iterations, { r = atan2f(y_val4, x_val4); });         \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "atan2f_quad4";                                                         \
    cnt++;                                                                                         \
  } while (0)

/* 其他数学函数 */                                                                                 \
#define TEST_SQRTF(result, iterations)                                                             \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "sqrtf", iterations, { r = sqrtf(sqrt_val); });              \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "sqrtf";                                                                \
    cnt++;                                                                                         \
  } while (0)

#define TEST_LOGF(result, iterations)                                                              \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "logf", iterations, { r = logf(sqrt_val); });                \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "logf";                                                                 \
    cnt++;                                                                                         \
  } while (0)

#define TEST_EXPF(result, iterations)                                                              \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "expf", iterations, { r = expf(sqrt_val); });                \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "expf";                                                                 \
    cnt++;                                                                                         \
  } while (0)

#define TEST_FAST_EXPF(result, iterations)                                                         \
  do {                                                                                             \
    MEASURE_TIME(result.cycles_total, "fast_expf", iterations, { r = fast_expf(sqrt_val); });      \
    result.ret           = r;                                                                      \
    result.cycles_per_op = result.cycles_total / iterations;                                       \
    result.name          = "fast_expf";                                                            \
    cnt++;                                                                                         \
  } while (0)

#define RUN_MATH_BENCHMARKS(results, iterations)                                                   \
  do {                                                                                             \
    volatile U32  cnt   = 0;                                                                       \
    volatile FP32 r     = 0;                                                                       \
    volatile I32  int_a = 123456, int_b = 789;                                                     \
    volatile FP32 float_a = 123.456f, float_b = 7.89f;                                             \
    volatile FP32 angle    = 0.785398f; /* 45 degrees in radians */                                \
    volatile FP32 sqrt_val = 2.0f;                                                                 \
    volatile FP32 y_val1 = 1.0f, x_val1 = 1.0f;                                                    \
    volatile FP32 y_val2 = 1.0f, x_val2 = -1.0f;                                                   \
    volatile FP32 y_val3 = -1.0f, x_val3 = -1.0f;                                                  \
    volatile FP32 y_val4 = -1.0f, x_val4 = 1.0f;                                                   \
    TEST_INT_ADD(results[cnt], iterations);                                                        \
    TEST_INT_MUL(results[cnt], iterations);                                                        \
    TEST_INT_DIV(results[cnt], iterations);                                                        \
    TEST_FLOAT_ADD(results[cnt], iterations);                                                      \
    TEST_FLOAT_MUL(results[cnt], iterations);                                                      \
    TEST_FLOAT_DIV(results[cnt], iterations);                                                      \
    TEST_SINF(results[cnt], iterations);                                                           \
    TEST_COSF(results[cnt], iterations);                                                           \
    TEST_TANF(results[cnt], iterations);                                                           \
    TEST_ARM_SINF(results[cnt], iterations);                                                       \
    TEST_ARM_COSF(results[cnt], iterations);                                                       \
    TEST_FAST_SINF(results[cnt], iterations);                                                      \
    TEST_FAST_COSF(results[cnt], iterations);                                                      \
    TEST_FAST_TANF(results[cnt], iterations);                                                      \
    TEST_ATAN2F_QUAD1(results[cnt], iterations);                                                   \
    TEST_ATAN2F_QUAD2(results[cnt], iterations);                                                   \
    TEST_ATAN2F_QUAD3(results[cnt], iterations);                                                   \
    TEST_ATAN2F_QUAD4(results[cnt], iterations);                                                   \
    TEST_SQRTF(results[cnt], iterations);                                                          \
    TEST_LOGF(results[cnt], iterations);                                                           \
    TEST_EXPF(results[cnt], iterations);                                                           \
    TEST_FAST_EXPF(results[cnt], iterations);                                                      \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif //! BENCHMARK_H
