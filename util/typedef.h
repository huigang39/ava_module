#ifndef TYPEDEF_H
#define TYPEDEF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char U8;
typedef signed char   I8;

typedef unsigned short U16;
typedef signed short   I16;

typedef unsigned int U32;
typedef signed int   I32;

//* on Windows, (unsigned long int) is 32bit
typedef unsigned long long U64;
typedef signed long long   I64;

typedef float  FP32;
typedef double FP64;

#ifdef __cplusplus
typedef bool BOOL;
#define TRUE  true
#define FALSE false
#else
typedef U8 BOOL;
#define TRUE  1
#define FALSE 0
#endif

typedef struct {
  U32 u;
  U32 v;
  U32 w;
} u32_uvw_t;

typedef struct {
  I32 u;
  I32 v;
  I32 w;
} i32_uvw_t;

typedef struct {
  FP32 u;
  FP32 v;
  FP32 w;
} fp32_uvw_t;

typedef struct {
  FP32 a;
  FP32 b;
} fp32_ab_t;

typedef struct {
  FP32 d;
  FP32 q;
} fp32_dq_t;

typedef struct {
  U32  npp;
  FP32 ld;
  FP32 lq;
  FP32 ls;
  FP32 rs;
} motor_param_t;

#ifdef __cplusplus
}
#endif

#endif // !TYPEDEF_H
