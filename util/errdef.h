#ifndef ERRDEF_H
#define ERRDEF_H

#ifdef __cpluscplus
extern "C" {
#endif

typedef enum {
  FAIL = -1,
  OK   = 0,
} ret_e;

#ifdef __cpluscplus
}
#endif

#endif // !ERRDEF_H
