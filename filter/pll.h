#ifndef PLL_H
#define PLL_H

#include "util/mathdef.h"
#include "util/typedef.h"

typedef struct {
  FP32 freq_hz;
  FP32 wc;
  FP32 fc;
  FP32 damp;
  FP32 kp;
  FP32 ki;
  FP32 filter_gain;
  FP32 filter_gain_ffd;
} pll_cfg_t;

typedef struct {
  fp32_ab_t val;
} pll_in_t;

typedef struct {
  FP32 theta_rad;
} pll_vel_in_t;

typedef struct {
  FP32 theta_rad;
  FP32 vel_rads;
  FP32 vel_rads_filter;
} pll_out_t;

typedef struct {
  FP32 err;
  FP32 err_theta_rad;
  FP32 ki_out;
  FP32 pi_out;
  FP32 pll_ffd;
} pll_lo_t;

typedef struct {
  FP32 err;
  FP32 err_theta_rad;
  FP32 prev_theta_rad;
  FP32 ki_out;
  FP32 pi_out;
  FP32 pll_ffd;
} pll_vel_lo_t;

typedef struct {
  pll_cfg_t cfg;
  pll_in_t  in;
  pll_out_t out;
  pll_lo_t  lo;
} pll_filter_t;

typedef struct {
  pll_cfg_t    cfg;
  pll_vel_in_t in;
  pll_out_t    out;
  pll_vel_lo_t lo;
} vel_pll_filter_t;

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

#define DECL_VEL_PLL_PTRS(pll)                                                                     \
  vel_pll_filter_t *p   = (pll);                                                                   \
  pll_cfg_t        *cfg = &p->cfg;                                                                 \
  pll_vel_in_t     *in  = &p->in;                                                                  \
  pll_out_t        *out = &p->out;                                                                 \
  pll_vel_lo_t     *lo  = &p->lo;

#define DECL_VEL_PLL_PTRS_PREFIX(pll, prefix)                                                      \
  vel_pll_filter_t *prefix##_p   = (pll);                                                          \
  pll_cfg_t        *prefix##_cfg = &prefix##_p->cfg;                                               \
  pll_vel_in_t     *prefix##_in  = &prefix##_p->in;                                                \
  pll_out_t        *prefix##_out = &prefix##_p->out;                                               \
  pll_vel_lo_t     *prefix##_lo  = &prefix##_p->lo;

static inline void
pll_init(pll_filter_t *pll, pll_cfg_t pll_cfg) {
  DECL_PLL_PTRS(pll);

  *cfg             = pll_cfg;
  cfg->kp          = FP32_2 * cfg->wc * cfg->damp;
  cfg->ki          = cfg->wc * cfg->wc * FP32_HZ_TO_S(cfg->freq_hz);
  cfg->filter_gain = FP32_1 / (FP32_1 + FP32_2PI * cfg->wc * FP32_HZ_TO_S(cfg->freq_hz));
  cfg->filter_gain_ffd
      = FP32_1 / (FP32_1 + FP32_2PI * cfg->wc * FP32_1_DIV_2 * FP32_HZ_TO_S(cfg->freq_hz));
}

static inline void
pll_run(pll_filter_t *pll) {
  DECL_PLL_PTRS(pll);

  lo->err = in->val.b * FP32_COS(out->theta_rad) - in->val.a * FP32_SIN(out->theta_rad);
  lo->ki_out += cfg->ki * lo->err;
  lo->pi_out    = cfg->kp * lo->err + lo->ki_out;
  out->vel_rads = lo->pi_out;
  out->theta_rad += out->vel_rads * FP32_HZ_TO_S(cfg->freq_hz);
  WARP_2PI(out->theta_rad);

  out->vel_rads_filter
      = cfg->filter_gain * out->vel_rads_filter + (FP32_1 - cfg->filter_gain) * out->vel_rads;
}

static inline void
pll_run_in(pll_filter_t *pll, fp32_ab_t ab) {
  DECL_PLL_PTRS(pll);

  in->val = ab;
  pll_run(pll);
}

static inline void
vel_pll_init(vel_pll_filter_t *vel_pll, pll_cfg_t pll_cfg) {
  DECL_VEL_PLL_PTRS(vel_pll);

  *cfg             = pll_cfg;
  cfg->kp          = FP32_2 * cfg->wc * cfg->damp;
  cfg->ki          = cfg->wc * cfg->wc * FP32_HZ_TO_S(cfg->freq_hz);
  cfg->filter_gain = FP32_1 / (FP32_1 + FP32_2PI * cfg->wc * FP32_HZ_TO_S(cfg->freq_hz));
  cfg->filter_gain_ffd
      = FP32_1 / (FP32_1 + FP32_2PI * cfg->wc * FP32_1_DIV_2 * FP32_HZ_TO_S(cfg->freq_hz));
}

static inline void
vel_pll_run(vel_pll_filter_t *pll) {
  DECL_VEL_PLL_PTRS(pll);

  lo->err_theta_rad = in->theta_rad - lo->prev_theta_rad;
  WARP_PI(lo->err_theta_rad);

  lo->pll_ffd
      = lo->pll_ffd * cfg->filter_gain_ffd
        + (lo->err_theta_rad / FP32_HZ_TO_S(cfg->freq_hz) * (FP32_1 - cfg->filter_gain_ffd));

  lo->err = in->theta_rad - out->theta_rad;
  WARP_PI(lo->err);

  lo->ki_out += cfg->ki * lo->err;
  lo->pi_out = cfg->kp * lo->err + lo->ki_out + lo->pll_ffd;

  out->vel_rads = lo->pi_out;
  out->vel_rads_filter
      = cfg->filter_gain * out->vel_rads_filter + (FP32_1 - cfg->filter_gain) * out->vel_rads;
  out->theta_rad += out->vel_rads * FP32_HZ_TO_S(cfg->freq_hz);
  WARP_2PI(out->theta_rad);

  lo->prev_theta_rad = in->theta_rad;
}

static inline void
vel_pll_run_in(vel_pll_filter_t *pll, FP32 theta_rad) {
  DECL_VEL_PLL_PTRS(pll);

  in->theta_rad = theta_rad;
  vel_pll_run(pll);
}

#endif // !PLL_H
