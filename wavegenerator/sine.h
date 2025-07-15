#ifndef SINE_H
#define SINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "util/mathdef.h"
#include "util/typedef.h"

typedef struct {
  FP32 freq_hz;
} sine_cfg_t;

typedef struct {
  FP32 freq_hz;
  FP32 amp_rad;
  FP32 phase_rad;
  FP32 offset_rad;
} sine_in_t;

typedef struct {
  FP32 val;
} sine_out_t;

typedef struct {
  FP32 phase_inc_rad;
} sine_lo_t;

typedef struct {
  sine_cfg_t cfg;
  sine_in_t  in;
  sine_out_t out;
  sine_lo_t  lo;
} sine_t;

#define DECL_SINE_PTRS(sine)                                                                       \
  sine_t     *p   = (sine);                                                                        \
  sine_cfg_t *cfg = &p->cfg;                                                                       \
  sine_in_t  *in  = &p->in;                                                                        \
  sine_out_t *out = &p->out;                                                                       \
  sine_lo_t  *lo  = &p->lo;

#define DECL_SINE_PTRS_PREFIX(sine, prefix)                                                        \
  sine_t     *prefix##_p   = (sine);                                                               \
  sine_cfg_t *prefix##_cfg = &prefix##_p->cfg;                                                     \
  sine_in_t  *prefix##_in  = &prefix##_p->in;                                                      \
  sine_out_t *prefix##_out = &prefix##_p->out;                                                     \
  sine_lo_t  *prefix##_lo  = &prefix##_p->lo;

static inline void
sine_init(sine_t *sine, sine_cfg_t sine_cfg) {
  DECL_SINE_PTRS(sine);

  *cfg = sine_cfg;
}

static inline void
sine_run(sine_t *sine) {
  DECL_SINE_PTRS(sine);

  lo->phase_inc_rad = FP32_2PI * in->freq_hz * FP32_HZ_TO_S(cfg->freq_hz);
  out->val          = in->amp_rad * FP32_SIN(in->phase_rad) + in->offset_rad;
  in->phase_rad += lo->phase_inc_rad;
  WARP_2PI(in->phase_rad);
}

#ifdef __cplusplus
}
#endif

#endif // !SINE_H