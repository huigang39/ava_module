#ifndef LPF_H
#define LPF_H

#ifdef __cpluscplus
extern "C" {
#endif

#include "module.h"

typedef struct {
  FP32 freq_hz;
} lpf_cfg_t;

typedef struct {
  FP32 val;
  FP32 fc;
} lpf_in_t;

typedef struct {
  FP32 val;
} lpf_out_t;

typedef struct {
  FP32 prev;
  FP32 curr;
  FP32 rc;
  FP32 alpha;
} lpf_lo_t;

typedef struct {
  lpf_cfg_t cfg;
  lpf_in_t  in;
  lpf_out_t out;
  lpf_lo_t  lo;
} lpf_filter_t;

#define DECL_LPF_PTRS(lpf)                                                                         \
  lpf_filter_t *p   = (lpf);                                                                       \
  lpf_cfg_t    *cfg = &p->cfg;                                                                     \
  lpf_in_t     *in  = &p->in;                                                                      \
  lpf_out_t    *out = &p->out;                                                                     \
  lpf_lo_t     *lo  = &p->lo;

#define DECL_LPF_PTRS_PREFIX(lpf, prefix)                                                          \
  lpf_filter_t *prefix##_p   = (lpf);                                                              \
  lpf_cfg_t    *prefix##_cfg = &prefix##_p->cfg;                                                   \
  lpf_in_t     *prefix##_in  = &prefix##_p->in;                                                    \
  lpf_out_t    *prefix##_out = &prefix##_p->out;                                                   \
  lpf_lo_t     *prefix##_lo  = &prefix##_p->lo;

static inline void
lpf_init(lpf_filter_t *lpf, lpf_cfg_t lpf_cfg) {
  DECL_LPF_PTRS(lpf);

  *cfg = lpf_cfg;
}

static inline void
lpf_run(lpf_filter_t *lpf) {
  DECL_LPF_PTRS(lpf);

  lo->rc    = FP32_1 / (FP32_2PI * in->fc);
  lo->alpha = cfg->freq_hz / (lo->rc + FP32_HZ_TO_S(cfg->freq_hz));
  lo->curr  = lo->alpha * lo->curr + (FP32_1 - lo->alpha) * lo->prev;
  lo->prev  = lo->curr;
}

#ifdef __cpluscplus
}
#endif

#endif // !LPF_H
