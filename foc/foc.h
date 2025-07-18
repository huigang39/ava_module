#ifndef FOC_H
#define FOC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "controller/pid.h"
#include "filter/pll.h"
#include "observer/smo.h"
#include "transform.h"
#include "util/mathdef.h"
#include "util/typedef.h"

typedef struct {
  i32_uvw_t i32_i_uvw, i32_v_uvw;
  I32       i32_v_bus;
} adc_raw_t;

typedef struct {
  FP32       v_max, v_min, v_avg;
  fp32_uvw_t fp32_pwm_duty;
  u32_uvw_t  u32_pwm_duty;
} svpwm_t;

typedef struct {
  FP32 theta_rad, vel_rads;
  FP32 sensor_theta_rad, sensor_vel_rads;
  FP32 obs_theta_rad, obs_vel_rads;
  FP32 force_theta_rad, force_vel_rads;
  FP32 mech_theta_rad;
} theta_t;

typedef struct {
  /* ADC */
  U32       adc_full_val;
  U32       adc_cail_cnt_max;
  FP32      cur_range, vbus_range;
  FP32      adc2cur, adc2vbus;
  adc_raw_t adc_offset;

  /* PWM */
  U32  pwm_freq_hz;
  U32  pwm_full_val;
  FP32 modulation_ratio;
  FP32 fp32_pwm_min, fp32_pwm_max;

  /* TIMER */
  U32 timer_freq_hz;
} periph_param_t;

typedef struct {
  FP32           freq_hz;
  FP32           theta_offset;
  BOOL           is_adc_cail;
  motor_param_t  motor;
  periph_param_t periph;
} foc_cfg_t;

typedef struct {
  adc_raw_t  adc_raw;
  theta_t    theta;
  fp32_uvw_t fp32_i_uvw, fp32_v_uvw;
  fp32_ab_t  i_ab, v_ab;
  fp32_dq_t  i_dq, v_dq;
} foc_in_t;

typedef struct {
  fp32_uvw_t fp32_i_uvw, fp32_v_uvw;
  fp32_ab_t  i_ab, v_ab;
  fp32_ab_t  v_ab_sv;
  fp32_dq_t  i_dq, v_dq;
  svpwm_t    svpwm;
} foc_out_t;

typedef enum {
  FOC_STATE_READY,
  FOC_STATE_DISABLE,
  FOC_STATE_ENABLE,
} foc_state_e;

typedef enum {
  FOC_THETA_NULL,
  FOC_THETA_FORCE,
  FOC_THETA_SENSOR,
  FOC_THETA_SENSORLESS,
  FOC_THETA_SENSORFUSION,
} foc_theta_e;

typedef struct {
  U32 NULL_FUNC_PTR : 1;
} foc_stat_t;

typedef struct {
  U64              exec_cnt;
  U32              elapsed;
  foc_stat_t       stat;
  U32              adc_cail_cnt;
  foc_state_e      e_state;
  foc_theta_e      e_theta;
  pid_ctrl_t       id_pid, iq_pid;
  vel_pll_filter_t vel_pll;
  smo_obs_t        smo;
} foc_lo_t;

typedef adc_raw_t (*foc_adc_get_f)(void);
typedef FP32 (*foc_theta_get_f)(void);
typedef void (*foc_pwm_set_f)(U32 pwm_full_val, u32_uvw_t u32_pwm_duty);
typedef void (*foc_drv_set_f)(U8 enable);

typedef struct {
  foc_adc_get_f   f_adc_get;
  foc_theta_get_f f_theta_get;
  foc_pwm_set_f   f_pwm_set;
  foc_drv_set_f   f_drv_set;
} foc_ops_t;

typedef struct {
  foc_cfg_t cfg;
  foc_in_t  in;
  foc_out_t out;
  foc_lo_t  lo;
  foc_ops_t ops;
} foc_t;

#define DECL_FOC_PTRS(foc)                                                                         \
  foc_t     *p   = (foc);                                                                          \
  foc_cfg_t *cfg = &p->cfg;                                                                        \
  foc_in_t  *in  = &p->in;                                                                         \
  foc_out_t *out = &p->out;                                                                        \
  foc_lo_t  *lo  = &p->lo;                                                                         \
  foc_ops_t *ops = &p->ops;

#define DECL_FOC_PTRS_PREFIX(foc, prefix)                                                          \
  foc_t     *prefix##_p   = (foc);                                                                 \
  foc_cfg_t *prefix##_cfg = &prefix##_p->cfg;                                                      \
  foc_in_t  *prefix##_in  = &prefix##_p->in;                                                       \
  foc_out_t *prefix##_out = &prefix##_p->out;                                                      \
  foc_lo_t  *prefix##_lo  = &prefix##_p->lo;                                                       \
  foc_ops_t *prefix##_ops = &prefix##_p->ops;

static inline void
svpwm(foc_t *foc) {
  DECL_FOC_PTRS(foc);

  out->fp32_v_uvw = inv_clarke(out->v_ab);

  if (out->fp32_v_uvw.u > out->fp32_v_uvw.v) {
    out->svpwm.v_max = out->fp32_v_uvw.u;
    out->svpwm.v_min = out->fp32_v_uvw.v;
  } else {
    out->svpwm.v_max = out->fp32_v_uvw.v;
    out->svpwm.v_min = out->fp32_v_uvw.u;
  }

  UPDATE_MIN_MAX(out->fp32_v_uvw.w, out->svpwm.v_min, out->svpwm.v_max);
  out->svpwm.v_avg = (out->svpwm.v_max + out->svpwm.v_min) * FP32_1_DIV_2;
  UVW_SUB_3ARG(out->svpwm.fp32_pwm_duty, out->fp32_v_uvw, out->svpwm.v_avg);

  UVW_ADD_2ARG(out->svpwm.fp32_pwm_duty, FP32_1_DIV_2);
  UVW_CLAMP(out->svpwm.fp32_pwm_duty, cfg->periph.fp32_pwm_min, cfg->periph.fp32_pwm_max);
  UVW_MUL_3ARG(out->svpwm.u32_pwm_duty, out->svpwm.fp32_pwm_duty, cfg->periph.pwm_full_val);
}

static inline void
foc_init(foc_t *foc, foc_cfg_t foc_cfg) {
  DECL_FOC_PTRS(foc);
  *cfg = foc_cfg;

  cfg->periph.adc2cur  = cfg->periph.cur_range / (FP32)cfg->periph.adc_full_val;
  cfg->periph.adc2vbus = cfg->periph.vbus_range / (FP32)cfg->periph.adc_full_val;

  pid_cfg_t pid_cfg;
  pid_cfg.freq_hz      = cfg->freq_hz;
  pid_cfg.kp           = 1500.0f * cfg->motor.ld;
  pid_cfg.ki           = 1500.0f * cfg->motor.rs;
  pid_cfg.out_max      = 48.0f / FP32_1_DIV_SQRT_3 * cfg->periph.fp32_pwm_max;
  pid_cfg.integral_max = pid_cfg.out_max;
  pid_init(&foc->lo.id_pid, pid_cfg);
  pid_init(&foc->lo.iq_pid, pid_cfg);

  pll_cfg_t pll_cfg;
  pll_cfg.freq_hz = cfg->freq_hz;
  pll_cfg.wc      = 200.0f;
  pll_cfg.fc      = 200.0f;
  pll_cfg.damp    = 0.707f;
  vel_pll_init(&foc->lo.vel_pll, pll_cfg);

  smo_cfg_t smo_cfg;
  smo_cfg.freq_hz = cfg->freq_hz;
  smo_cfg.motor   = cfg->motor;
  smo_cfg.kp      = 10.0f;
  smo_cfg.es0     = 500.0f;
  smo_init(&foc->lo.smo, smo_cfg);
}

static inline void
foc_ready(foc_t *foc) {
  DECL_FOC_PTRS(foc);

  if (cfg->is_adc_cail)
    return;

  in->adc_raw = ops->f_adc_get();
  UVW_ADD_UVW(cfg->periph.adc_offset.i32_i_uvw, in->adc_raw.i32_i_uvw);
  if (++lo->adc_cail_cnt >= LF(cfg->periph.adc_cail_cnt_max)) {
    SELF_RF(cfg->periph.adc_offset.i32_i_uvw.u, cfg->periph.adc_cail_cnt_max);
    SELF_RF(cfg->periph.adc_offset.i32_i_uvw.v, cfg->periph.adc_cail_cnt_max);
    SELF_RF(cfg->periph.adc_offset.i32_i_uvw.w, cfg->periph.adc_cail_cnt_max);
    cfg->is_adc_cail = TRUE;
  }
}

static inline void
foc_enable(foc_t *foc) {
  DECL_FOC_PTRS(foc);

  ops->f_drv_set(TRUE);
}

static inline void
foc_disable(foc_t *foc) {
  DECL_FOC_PTRS(foc);

  ops->f_drv_set(FALSE);
}

static inline void
foc_run(foc_t *foc) {
  DECL_FOC_PTRS(foc);

  lo->exec_cnt++;

  in->adc_raw = ops->f_adc_get();
  UVW_SUB_UVW(in->adc_raw.i32_i_uvw, cfg->periph.adc_offset.i32_i_uvw);
  UVW_MUL_3ARG(in->fp32_i_uvw, in->adc_raw.i32_i_uvw, cfg->periph.adc2cur);

  in->theta.mech_theta_rad   = ops->f_theta_get();
  in->theta.sensor_theta_rad = MECH_TO_ELEC(in->theta.mech_theta_rad, cfg->motor.npp);
  WARP_2PI(in->theta.sensor_theta_rad);

  DECL_VEL_PLL_PTRS_PREFIX(&foc->lo.vel_pll, vel_pll)
  vel_pll_run_in(vel_pll_p, in->theta.sensor_theta_rad);
  in->theta.sensor_vel_rads = vel_pll_out->vel_rads_filter;

  DECL_SMO_PTRS_PREFIX(&foc->lo.smo, smo);
  smo_run_in(smo_p, in->i_ab, out->v_ab);
  in->theta.obs_theta_rad = smo_out->theta_rad;
  in->theta.obs_vel_rads  = smo_out->vel_rads;

  switch (lo->e_theta) {
  case FOC_THETA_FORCE:
    in->theta.theta_rad = in->theta.force_theta_rad;
    in->theta.vel_rads  = in->theta.force_vel_rads;
    break;
  case FOC_THETA_SENSOR:
    in->theta.theta_rad = in->theta.sensor_theta_rad;
    in->theta.vel_rads  = in->theta.sensor_vel_rads;
    break;
  case FOC_THETA_SENSORLESS:
    in->theta.theta_rad = in->theta.obs_theta_rad;
    in->theta.vel_rads  = in->theta.obs_vel_rads;
    break;
  case FOC_THETA_SENSORFUSION:
    break;
  default:
    break;
  }

  switch (lo->e_state) {
  case FOC_STATE_READY:
    foc_ready(foc);
    return;
  case FOC_STATE_DISABLE:
    foc_disable(foc);
    return;
  case FOC_STATE_ENABLE:
    foc_enable(foc);
    break;
  default:
    return;
  }

  // only FOC_STATE_ENABLE can run below code!!!
  in->i_ab = clarke(in->fp32_i_uvw, cfg->periph.modulation_ratio);
  in->i_dq = park(in->i_ab, in->theta.theta_rad);

  DECL_PID_PTRS_PREFIX(&foc->lo.id_pid, id_pid);
  pid_run_in(id_pid, out->i_dq.d, in->i_dq.d);
  out->v_dq.d = id_pid_out->val;

  DECL_PID_PTRS_PREFIX(&foc->lo.iq_pid, iq_pid);
  pid_run_in(&foc->lo.iq_pid, out->i_dq.q, in->i_dq.q);
  out->v_dq.q = iq_pid_out->val;

  out->v_ab      = inv_park(out->v_dq, in->theta.theta_rad);
  out->v_ab_sv.a = out->v_ab.a / 48.0f;
  out->v_ab_sv.b = out->v_ab.b / 48.0f;
  svpwm(foc);
  ops->f_pwm_set(cfg->periph.pwm_full_val, out->svpwm.u32_pwm_duty);
}

#ifdef __cplusplus
}
#endif

#endif // !FOC_H
