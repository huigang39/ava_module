#ifndef PLL_H
#define PLL_H

#include "util/mathdef.h"
#include "util/typedef.h"

typedef struct {
  FP32 freq_hz;
  FP32 wc;
  FP32 fc;
  FP32 damp;
} pll_cfg_t;

typedef struct {
  fp32_ab_t val;
  FP32      theta_rad;
} pll_in_t;

typedef struct {
  FP32 theta_rad;
  FP32 vel_rads;
  FP32 vel_rads_filter;
} pll_out_t;

typedef struct {
  FP32 err;
  FP32 err_theta_rad;
  FP32 prev_theta_rad;
  FP32 ki_out;
  FP32 pi_out;
  FP32 kp;
  FP32 ki;
  FP32 gain;
  FP32 gain_ffd;
  FP32 pll_ffd;
} pll_lo_t;

typedef struct {
  pll_cfg_t cfg;
  pll_in_t  in;
  pll_out_t out;
  pll_lo_t  lo;
} pll_filter_t;

#define DECL_PLL_PTRS(pll)                                                                         \
  pll_filter_t *p   = (pll);                                                                       \
  pll_cfg_t    *cfg = &p->cfg;                                                                     \
  pll_in_t     *in  = &p->in;                                                                      \
  pll_out_t    *out = &p->out;                                                                     \
  pll_lo_t     *lo  = &p->lo;

#define DECL_PLL_PTRS_PREFIX(pll, prefix)                                                          \
  pll_filter_t *prefix##_p   = (pll);                                                              \
  pll_cfg_t    *prefix##_cfg = &prefix##_p->cfg;                                                   \
  pll_in_t     *prefix##_in  = &prefix##_p->in;                                                    \
  pll_out_t    *prefix##_out = &prefix##_p->out;                                                   \
  pll_lo_t     *prefix##_lo  = &prefix##_p->lo;

static inline void
pll_init(pll_filter_t *pll, pll_cfg_t pll_cfg) {
  DECL_PLL_PTRS(pll);

  *cfg = pll_cfg;
}

static inline void
pll_run(pll_filter_t *pll) {
  DECL_PLL_PTRS(pll);

  lo->kp       = FP32_2 * cfg->wc * cfg->damp;
  lo->ki       = cfg->wc * cfg->wc * FP32_HZ_TO_S(cfg->freq_hz);
  lo->gain     = FP32_1 / (FP32_1 + FP32_2PI * cfg->wc * FP32_HZ_TO_S(cfg->freq_hz));
  lo->gain_ffd = FP32_1 / (FP32_1 + FP32_2PI * cfg->wc * FP32_1_DIV_2 * FP32_HZ_TO_S(cfg->freq_hz));
  lo->err      = in->val.b * FP32_COS(out->theta_rad) - in->val.a * FP32_SIN(out->theta_rad);
  lo->ki_out += lo->ki * lo->err;
  lo->pi_out    = lo->kp * lo->err + lo->ki_out;
  out->vel_rads = lo->pi_out;
  out->theta_rad += out->vel_rads * FP32_HZ_TO_S(cfg->freq_hz);
  WARP2_PI(out->theta_rad);

  out->vel_rads_filter = lo->gain * out->vel_rads_filter + (FP32_1 - lo->gain) * out->vel_rads;
}

static inline void
pll_run_in(pll_filter_t *pll, FP32 alpha, FP32 beta) {
  DECL_PLL_PTRS(pll);

  in->val.a = alpha;
  in->val.b = beta;
  pll_run(pll);
}

static inline void
pll_vel_run(pll_filter_t *pll) {
  DECL_PLL_PTRS(pll);

  lo->err_theta_rad = in->theta_rad - lo->prev_theta_rad;
  WARP_PI(lo->err_theta_rad);

  lo->pll_ffd = lo->pll_ffd * lo->gain_ffd
                + (lo->err_theta_rad / FP32_HZ_TO_S(cfg->freq_hz) * (FP32_1 - lo->gain_ffd));

  lo->err = in->theta_rad - out->theta_rad;
  WARP_PI(lo->err);

  lo->ki_out += lo->ki * lo->err;
  lo->pi_out = lo->kp * lo->err + lo->ki_out + lo->pll_ffd;

  out->vel_rads        = lo->pi_out;
  out->vel_rads_filter = lo->gain * out->vel_rads_filter + (FP32_1 - lo->gain) * out->vel_rads;
  out->theta_rad += out->vel_rads * FP32_HZ_TO_S(cfg->freq_hz);
  WARP_2PI(out->theta_rad);

  lo->prev_theta_rad = in->theta_rad;
}

static inline void
pll_vel_run_in(pll_filter_t *pll, FP32 theta_rad) {
  DECL_PLL_PTRS(pll);

  in->theta_rad = theta_rad;
  pll_vel_run(pll);
}

#endif // !PLL_H
