#ifndef FASTMATH_H
#define FASTMATH_H

#ifdef __cpluscplus
extern "C" {
#endif

#include "typedef.h"

#define FP32_HOLLYST 0.017453292519943295769236907684886F

static const FP32 SIN_TABLE[] = {
  0.0f,                                // sin(0)
  0.17364817766693034885171662676931f, // sin(10)
  0.34202014332566873304409961468226f, // sin(20)
  0.5f,                                // sin(30)
  0.64278760968653932632264340990726f, // sin(40)
  0.76604444311897803520239265055542f, // sin(50)
  0.86602540378443864676372317075294f, // sin(60)
  0.93969262078590838405410927732473f, // sin(70)
  0.98480775301220805936674302458952f, // sin(80)
  1.0f                                 // sin(90)
};

static const FP32 COS_TABLE[] = {
  1.0f,                                // cos(0)
  0.99984769515639123915701155881391f, // cos(10)
  0.99939082701909573000624344004393f, // cos(20)
  0.99862953475457387378449205843944f, // cos(30)
  0.99756405025982424761316268064426f, // cos(40)
  0.99619469809174553229501040247389f, // cos(50)
  0.99452189536827333692269194498057f, // cos(60)
  0.99254615164132203498006158933058f, // cos(70)
  0.99026806874157031508377486734485f, // cos(80)
  0.98768834059513772619004024769344f  // cos(90)
};

static inline FP32
fast_sinf(FP32 x) {
  x = (x) * (57.295779513082320876798154814105f);

  I32 sig = 0u;
  if (x > 0.0f) {
    while (x >= 360.0f)
      x = x - 360.0f;
  } else {
    while (x < 0.0f)
      x = x + 360.0f;
  }

  if (x >= 180.0f) {
    sig = 1u;
    x   = x - 180.0f;
  }

  x = (x > 90.0f) ? (180.0f - x) : x;

  I32  a = x * 0.1f;
  FP32 b = x - 10.0f * a;

  FP32 y = SIN_TABLE[a] * COS_TABLE[(I32)b] + b * FP32_HOLLYST * SIN_TABLE[9u - a];
  return (sig > 0u) ? -y : y;
}

static inline FP32
fast_cosf(FP32 x) {
  return fast_sinf(x + 1.5707963267948966192313216916398f);
}

static inline FP32
fast_tanf(FP32 x) {
  return fast_sinf(x) / fast_cosf(x);
}

static inline FP32
fast_expf(FP32 x) {
  union {
    U32  i;
    FP32 f;
  } v;
  v.i = (1u << 23u) * (1.4426950409f * x + 126.93490512f);
  return v.f;
}

static inline FP32
fast_absf(FP32 x) {
  FP32 y = x;

  if (x < (float)0.0f)
    y = -x;

  return y;
}

#ifdef __cpluscplus
}
#endif

#endif // !FASTMATH_H
