#ifndef PID_H
#define PID_H

#ifdef __cpluscplus
extern "C" {
#endif

#include "util/mathdef.h"
#include "util/typedef.h"

typedef struct {
  FP32 freq_hz;
  FP32 kp;
  FP32 ki;
  FP32 kd;
  FP32 out_max;
  FP32 integral_max;
} pid_cfg_t;

typedef struct {
  FP32 ref;
  FP32 fdb;
} pid_in_t;

typedef struct {
  FP32 val;
} pid_out_t;

typedef struct {
  FP32 err;
  FP32 prev_err;
  FP32 kp_out;
  FP32 ki_out;
  FP32 kd_out;
} pid_lo_t;

typedef struct {
  pid_cfg_t cfg;
  pid_in_t  in;
  pid_out_t out;
  pid_lo_t  lo;
} pid_ctrl_t;

#define DECL_PID_PTRS(pid)                                                                         \
  pid_ctrl_t *p   = (pid);                                                                         \
  pid_cfg_t  *cfg = &p->cfg;                                                                       \
  pid_in_t   *in  = &p->in;                                                                        \
  pid_out_t  *out = &p->out;                                                                       \
  pid_lo_t   *lo  = &p->lo;

#define DECL_PID_PTRS_PREFIX(pid, prefix)                                                          \
  pid_ctrl_t *prefix       = (pid);                                                                \
  pid_cfg_t  *prefix##_cfg = &prefix->cfg;                                                         \
  pid_in_t   *prefix##_in  = &prefix->in;                                                          \
  pid_out_t  *prefix##_out = &prefix->out;                                                         \
  pid_lo_t   *prefix##_lo  = &prefix->lo;

static inline void
pid_init(pid_ctrl_t *pid, pid_cfg_t pid_cfg) {
  DECL_PID_PTRS(pid);

  *cfg = pid_cfg;
}

static inline void
pid_run(pid_ctrl_t *pid) {
  DECL_PID_PTRS(pid);

  lo->err = in->ref - in->fdb;

  lo->kp_out = cfg->kp * lo->err;
  lo->ki_out += cfg->ki * lo->err * FP32_HZ_TO_S(cfg->freq_hz);
  CLAMP(lo->ki_out, -cfg->integral_max, cfg->integral_max);
  lo->kd_out = cfg->kd * (lo->err - lo->prev_err) / FP32_HZ_TO_S(cfg->freq_hz);

  out->val = lo->kp_out + lo->ki_out + lo->kd_out;
  CLAMP(out->val, -cfg->out_max, cfg->out_max);

  lo->prev_err = lo->err;
}

static inline void
pid_run_in(pid_ctrl_t *pid, FP32 ref, FP32 fdb) {
  DECL_PID_PTRS(pid);
  in->ref = ref;
  in->fdb = fdb;
  pid_run(pid);
}

#ifdef __cpluscplus
}
#endif

#endif // !PID_H
