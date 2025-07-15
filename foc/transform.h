#ifndef TRANSFORM_H
#define TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/typedef.h"

static inline fp32_ab_t
clarke(fp32_uvw_t fp32_abc, FP32 mode_rate) {
  fp32_ab_t fp32_ab;
  fp32_ab.a = mode_rate * (fp32_abc.u - FP32_1_DIV_2 * (fp32_abc.v + fp32_abc.w));
  fp32_ab.b = mode_rate * (fp32_abc.v - fp32_abc.w) * FP32_SQRT_3_DIV_2;
  return fp32_ab;
}

static inline fp32_uvw_t
inv_clarke(fp32_ab_t fp32_ab) {
  fp32_uvw_t fp32_uvw;
  FP32       fp32_a = -(fp32_ab.a * FP32_1_DIV_2);
  FP32       fp32_b = fp32_ab.b * FP32_SQRT_3_DIV_2;
  fp32_uvw.u        = fp32_ab.a;
  fp32_uvw.v        = fp32_a + fp32_b;
  fp32_uvw.w        = fp32_a - fp32_b;
  return fp32_uvw;
}

static inline fp32_dq_t
park(fp32_ab_t fp32_ab, FP32 theta) {
  fp32_dq_t fp32_dq;
  fp32_dq.d = FP32_COS(theta) * fp32_ab.a + FP32_SIN(theta) * fp32_ab.b;
  fp32_dq.q = FP32_COS(theta) * fp32_ab.b - FP32_SIN(theta) * fp32_ab.a;
  return fp32_dq;
}

static inline fp32_ab_t
inv_park(fp32_dq_t fp32_dq, FP32 theta) {
  fp32_ab_t fp32_ab;
  fp32_ab.a = FP32_COS(theta) * fp32_dq.d - FP32_SIN(theta) * fp32_dq.q;
  fp32_ab.b = FP32_SIN(theta) * fp32_dq.d + FP32_COS(theta) * fp32_dq.q;
  return fp32_ab;
}

#ifdef __cplusplus
}
#endif

#endif // !TRANSFORM_H
