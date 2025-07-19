#ifndef FFT_H
#define FFT_H

#ifdef __cpluscplus
extern "C" {
#endif

#ifndef FFT_POINT_SIZE
#define FFT_POINT_SIZE (LF(6))
#endif

#ifdef ARM_MATH
#include "arm_const_structs.h"
#include "arm_math.h"
#endif

#include "util/mathdef.h"
#include "util/typedef.h"

typedef struct {
  FP32                       sample_rate_hz;
  U8                         flag;
  arm_rfft_fast_instance_f32 S;
} fft_cfg_t;

typedef struct {
  FP32 buf[FFT_POINT_SIZE];
} fft_in_t;

typedef struct {
  FP32 buf[FFT_POINT_SIZE];
  FP32 module[FFT_POINT_SIZE];
  U32  idx;
  FP32 value_max;
  FP32 freq_hz_max;
} fft_out_t;

typedef struct {
  FP32 buf[FFT_POINT_SIZE];
  U32  idx_added;
} fft_lo_t;

typedef struct {
  fft_cfg_t cfg;
  fft_in_t  in;
  fft_out_t out;
  fft_lo_t  lo;
} fft_t;

#define DECL_FFT_PTRS(fft)                                                                         \
  fft_t     *p   = (fft);                                                                          \
  fft_cfg_t *cfg = &p->cfg;                                                                        \
  fft_in_t  *in  = &p->in;                                                                         \
  fft_out_t *out = &p->out;                                                                        \
  fft_lo_t  *lo  = &p->lo;

#define DECL_FFT_PTRS_PREFIX(fft, prefix)                                                          \
  fft_t     *prefix##_p   = (fft);                                                                 \
  fft_cfg_t *prefix##_cfg = &prefix##_p->cfg;                                                      \
  fft_in_t  *prefix##_in  = &prefix##_p->in;                                                       \
  fft_out_t *prefix##_out = &prefix##_p->out;                                                      \
  fft_lo_t  *prefix##_lo  = &prefix##_p->lo;

static inline void
fft_init(fft_t *fft, fft_cfg_t fft_cfg) {
  DECL_FFT_PTRS(fft);

  *cfg      = fft_cfg;
  cfg->flag = 0u;
  switch (FFT_POINT_SIZE) {
  case LF(5):
    arm_rfft_fast_init_32_f32(&cfg->S);
    break;
  case LF(6):
    arm_rfft_fast_init_64_f32(&cfg->S);
    break;
  case LF(7):
    arm_rfft_fast_init_128_f32(&cfg->S);
    break;
  case LF(8):
    arm_rfft_fast_init_256_f32(&cfg->S);
    break;
  case LF(9):
    arm_rfft_fast_init_512_f32(&cfg->S);
    break;
  case LF(10):
    arm_rfft_fast_init_1024_f32(&cfg->S);
    break;
  case LF(11):
    arm_rfft_fast_init_2048_f32(&cfg->S);
    break;
  case LF(12):
    arm_rfft_fast_init_4096_f32(&cfg->S);
    break;
  default:
    break;
  }
}

static inline void
fft_add_value(fft_t *fft, FP32 value) {
  DECL_FFT_PTRS(fft);

  lo->idx_added            = lo->idx_added >= FFT_POINT_SIZE ? 0 : lo->idx_added;
  in->buf[lo->idx_added++] = value;
}

static inline void
fft_run(fft_t *fft) {
  DECL_FFT_PTRS(fft);

  memcpy(lo->buf, in->buf, sizeof(lo->buf));
  arm_rfft_fast_f32(&cfg->S, lo->buf, out->buf, cfg->flag);
  arm_cmplx_mag_f32(out->buf, out->module, FFT_POINT_SIZE / 2);
  arm_max_f32(&out->module[1], FFT_POINT_SIZE / 2, &out->value_max, &out->idx);
  out->freq_hz_max = out->idx * cfg->sample_rate_hz / FFT_POINT_SIZE;
}

static inline void
fft_run_in(fft_t *fft, FP32 *buf) {
  DECL_FFT_PTRS(fft);

  memcpy(in->buf, buf, sizeof(in->buf));
  fft_run(fft);
}

#ifdef __cpluscplus
}
#endif

#endif // !FFT_H
