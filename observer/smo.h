#ifndef SMO_H
#define SMO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "filter/pll.h"
#include "util/typedef.h"

typedef struct {
  FP32          freq_hz;
  motor_param_t motor;
  FP32          kp;
  FP32          es0;
} smo_cfg_t;

typedef struct {
  fp32_ab_t i_ab, v_ab;
} smo_in_t;

typedef struct {
  fp32_ab_t v_ab_emf, i_ab_obs;
  FP32      theta_rad;
  FP32      vel_rads;
} smo_out_t;

typedef struct {
  pll_filter_t pll;
  fp32_ab_t    i_ab_obs_err;
} smo_lo_t;

typedef struct {
  smo_cfg_t cfg;
  smo_in_t  in;
  smo_out_t out;
  smo_lo_t  lo;
} smo_obs_t;

#define DECL_SMO_PTRS(smo)                                                                         \
  smo_obs_t *p   = (smo);                                                                          \
  smo_cfg_t *cfg = &p->cfg;                                                                        \
  smo_in_t  *in  = &p->in;                                                                         \
  smo_out_t *out = &p->out;                                                                        \
  smo_lo_t  *lo  = &p->lo;

#define DECL_SMO_PTRS_PREFIX(smo, prefix)                                                          \
  smo_obs_t *prefix##_p   = (smo);                                                                 \
  smo_cfg_t *prefix##_cfg = &prefix##_p->cfg;                                                      \
  smo_in_t  *prefix##_in  = &prefix##_p->in;                                                       \
  smo_out_t *prefix##_out = &prefix##_p->out;                                                      \
  smo_lo_t  *prefix##_lo  = &prefix##_p->lo;

static inline void
smo_init(smo_obs_t *smo, smo_cfg_t smo_cfg) {
  DECL_SMO_PTRS(smo);

  *cfg = smo_cfg;

  pll_cfg_t pll_cfg;
  pll_cfg.freq_hz = cfg->freq_hz;
  pll_cfg.wc      = 500.0f;
  pll_cfg.fc      = 200.0f;
  pll_cfg.damp    = 0.707f;
  pll_init(&smo->lo.pll, pll_cfg);
}

static inline void
smo_run(smo_obs_t *smo) {
  DECL_SMO_PTRS(smo);
  DECL_PLL_PTRS_PREFIX(&smo->lo.pll, pll);

  out->theta_rad = pll_out->theta_rad;
  pll_run_in(pll_p, out->v_ab_emf);
  out->vel_rads = pll_out->vel_rads;

  AB_SUB_3ARG(lo->i_ab_obs_err, out->i_ab_obs, in->i_ab);

  FP32 sign_alpha
      = (lo->i_ab_obs_err.a > cfg->es0)
            ? cfg->kp
            : ((lo->i_ab_obs_err.a < -cfg->es0) ? -cfg->kp
                                                : (cfg->kp * lo->i_ab_obs_err.a / cfg->es0));
  FP32 sign_beta
      = (lo->i_ab_obs_err.b > cfg->es0)
            ? cfg->kp
            : ((lo->i_ab_obs_err.b < -cfg->es0) ? -cfg->kp
                                                : (cfg->kp * lo->i_ab_obs_err.b / cfg->es0));

  out->v_ab_emf.a = sign_alpha;
  out->v_ab_emf.b = sign_beta;

  out->i_ab_obs.a += (in->v_ab.a - in->i_ab.a * cfg->motor.rs - out->v_ab_emf.a)
                     * FP32_HZ_TO_S(cfg->freq_hz) / cfg->motor.ls;

  out->i_ab_obs.b += (in->v_ab.b - in->i_ab.b * cfg->motor.rs - out->v_ab_emf.b)
                     * FP32_HZ_TO_S(cfg->freq_hz) / cfg->motor.ls;
}

static inline void
smo_run_in(smo_obs_t *smo, fp32_ab_t i_ab, fp32_ab_t v_ab) {
  DECL_SMO_PTRS(smo);

  in->i_ab = i_ab;
  in->v_ab = v_ab;
  smo_run(smo);
}

#ifdef __cplusplus
}
#endif

#endif // !SMO_H
