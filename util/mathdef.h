#ifndef MATHDEF_H
#define MATHDEF_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM_MATH
#include "arm_math.h"
#endif

#include <math.h>

#include "fastmath.h"
#include "typedef.h"

#ifdef FAST_MATH
#define FP32_SIN(x)    fast_sinf(x)
#define FP32_COS(x)    fast_cosf(x)
#define FP32_TAN(x)    fast_tanf(x)
#define FP32_EXP(x)    fast_expf(x)
#define FP32_ABS(x)    fast_absf(x)
#define FP32_MOD(x, y) fmodf(x, y) // __hardfp_fmodf
#elif defined(ARM_MATH)
#define FP32_SIN(x)         arm_sin_f32(x)
#define FP32_COS(x)         arm_cos_f32(x)
#define FP32_ABS(x)         arm_abs_f32(x)
#define FP32_ATAN2(y, x, r) arm_atan2_f32(y, x, r)
#define FP32_EXP(x)         fast_expf(x)
#else
#define FP32_SIN(x)      sinf(x)
#define FP32_COS(x)      cosf(x)
#define FP32_EXP(x)      expf(x)
#define FP32_ATAN2(y, x) atan2f(y, x)
#define FP32_ABS(x)      fabsf(x)
#define FP32_MOD(x, y)   fmodf(x, y) // __hardfp_fmodf
#endif

#define FP32_PI                  (3.1415926535897932384626433832795F)
#define FP32_2PI                 (6.283185307179586476925286766559F)
#define FP32_PI_DIV_2            (1.5707963267948966192313216916398F)
#define FP32_E                   (2.7182818284590452353602874713527F)
#define FP32_LN2                 (0.69314718055994530941723212145818F)

#define FP32_0                   (0.0F)
#define FP32_1                   (1.0F)
#define FP32_2                   (2.0F)
#define FP32_1_DIV_2             (0.5F)
#define FP32_2_DIV_3             (0.66666666666666666666666666666667F)
#define FP32_SQRT_2              (1.4142135623730950488016887242097F)
#define FP32_SQRT_3              (1.7320508075688772935274463415059F)
#define FP32_1_DIV_SQRT_3        (0.57735026918962576450914878050196F)
#define FP32_SQRT_3_DIV_2        (0.86602540378443864676372317075294F)

#define FP32_K                   (1E3F)
#define FP32_M                   (1E6F)
#define FP32_G                   (1E9F)

#define U32_K                    (1000U)
#define U32_M                    (1000000U)
#define U32_G                    (1000000000U)

#define FP32_MUL_K(val)          ((val) * (FP32_K))
#define FP32_MUL_M(val)          ((val) * (FP32_M))
#define FP32_MUL_G(val)          ((val) * (FP32_G))

#define U32_MUL_K(val)           ((val) * (U32_K))
#define U32_MUL_M(val)           ((val) * (U32_M))
#define U32_MUL_G(val)           ((val) * (U32_G))

#define U32_LF(n)                ((1U) << (n))
#define U32_RF(n)                ((1U) >> (n))

#define MIN(x, y)                ((x) < (y) ? (x) : (y))

#define RAD_TO_DEG(rad)          ((rad) * (57.295779513082320876798154814105F))
#define DEG_TO_RAD(deg)          ((deg) / (57.295779513082320876798154814105F))

#define FP32_US_TO_S(us)         ((us) / (FP32_M))

#define FP32_HZ_TO_S(hz)         ((FP32_1) / (hz))
#define FP32_HZ_TO_MS(hz)        ((FP32_1) / (hz) * (FP32_K))
#define FP32_HZ_TO_US(hz)        ((FP32_1) / (hz) * (FP32_M))

#define U32_HZ_TO_S(hz)          (U32)(FP32_HZ_TO_S(hz))
#define U32_HZ_TO_MS(hz)         (U32)(FP32_HZ_TO_MS(hz))
#define U32_HZ_TO_US(hz)         (U32)(FP32_HZ_TO_US(hz))

#define MECH_TO_ELEC(theta, npp) ((theta) * (npp))
#define ELEC_TO_MECH(theta, npp) ((theta) / (npp))

#define U32_TO_LF(val, n)                                                                          \
  do {                                                                                             \
    (val) <<= (n);                                                                                 \
  } while (0)

#define U32_TO_RF(val, n)                                                                          \
  do {                                                                                             \
    (val) >>= (n);                                                                                 \
  } while (0)

#define CLAMP(val, min, max)                                                                       \
  do {                                                                                             \
    if ((val) < (min))                                                                             \
      (val) = (min);                                                                               \
    else if ((val) > (max))                                                                        \
      (val) = (max);                                                                               \
  } while (0)

#define UVW_CLAMP(val, min, max)                                                                   \
  do {                                                                                             \
    CLAMP((val).u, (min), (max));                                                                  \
    CLAMP((val).v, (min), (max));                                                                  \
    CLAMP((val).w, (min), (max));                                                                  \
  } while (0)

#define UPDATE_MIN_MAX(val, min, max)                                                              \
  do {                                                                                             \
    if ((val) < (min))                                                                             \
      (min) = (val);                                                                               \
    else if ((val) > (max))                                                                        \
      (max) = (val);                                                                               \
  } while (0)

#define WARP_PI(rad)                                                                               \
  do {                                                                                             \
    if (FP32_ABS(rad) > FP32_2PI)                                                                  \
      rad = FP32_MOD(rad, FP32_2PI);                                                               \
    if (rad > FP32_PI)                                                                             \
      rad -= FP32_2PI;                                                                             \
    else if (rad < -FP32_PI)                                                                       \
      rad += FP32_2PI;                                                                             \
  } while (0)

#define WARP_2PI(rad)                                                                              \
  do {                                                                                             \
    if (FP32_ABS(rad) > FP32_2PI)                                                                  \
      rad = FP32_MOD(rad, FP32_2PI);                                                               \
    if (rad < FP32_0)                                                                              \
      rad += FP32_2PI;                                                                             \
  } while (0)

#define UVW_ADD_UVW(x, y)                                                                          \
  do {                                                                                             \
    (x).u = (x).u + (y).u;                                                                         \
    (x).v = (x).v + (y).v;                                                                         \
    (x).w = (x).w + (y).w;                                                                         \
  } while (0)

#define UVW_ADD_2ARG(x, y)                                                                         \
  do {                                                                                             \
    (x).u = (x).u + (y);                                                                           \
    (x).v = (x).v + (y);                                                                           \
    (x).w = (x).w + (y);                                                                           \
  } while (0)

#define UVW_SUB_UVW(x, y)                                                                          \
  do {                                                                                             \
    (x).u = (x).u - (y).u;                                                                         \
    (x).v = (x).v - (y).v;                                                                         \
    (x).w = (x).w - (y).w;                                                                         \
  } while (0)

#define UVW_SUB_3ARG(val, x, y)                                                                    \
  do {                                                                                             \
    (val).u = (x).u - (y);                                                                         \
    (val).v = (x).v - (y);                                                                         \
    (val).w = (x).w - (y);                                                                         \
  } while (0)

#define UVW_MUL_UVW(x, y)                                                                          \
  do {                                                                                             \
    (x).u = (x).u * (y).u;                                                                         \
    (x).v = (x).v * (y).v;                                                                         \
    (x).w = (x).w * (y).w;                                                                         \
  } while (0)

#define UVW_MUL_2ARG(x, y)                                                                         \
  do {                                                                                             \
    (x).u = (x).u * (y);                                                                           \
    (x).v = (x).v * (y);                                                                           \
    (x).w = (x).w * (y);                                                                           \
  } while (0)

#define UVW_MUL_3ARG(val, x, y)                                                                    \
  do {                                                                                             \
    (val).u = (x).u * (y);                                                                         \
    (val).v = (x).v * (y);                                                                         \
    (val).w = (x).w * (y);                                                                         \
  } while (0)

#define AB_SUB_3ARG(val, x, y)                                                                     \
  do {                                                                                             \
    (val).a = (x).a - (y).a;                                                                       \
    (val).b = (x).b - (y).b;                                                                       \
  } while (0)

#ifdef __cplusplus
}
#endif

#endif // !MATHDEF_H
