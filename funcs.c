/*===========================================================================

  kcalc-cpm

  funcs.c

  Note: I've tried to capture common math errors in these functions,
  like division by zero, but I'm not sure I've caught them all. The 
  Aztec C library provides no math error detection, so any I haven't 
  handled will result in gibberish results.

  Copyright (c)2021 Kevin Boone

===========================================================================*/

#include "setjmp.h"
#include "tinyexpr.h"
#include "math.h"
#include "funcs.h"

/* We have to use longjmp to handle errors, as the tinyexpr library provides
   no exception-handling or error reporting method. */
extern jmp_buf err_jump;

double DEG_TO_RAD = 2.0 * CONST_PI / 360.0;
double RAD_TO_DEG = 360.0 / 2.0 / CONST_PI;

/** atan */
double _atan (a) 
double a; 
  {
  if (angle_mode == AM_DEG)
    return RAD_TO_DEG * atan (a);
  else
    return atan (a); 
  }

/** sqrt with error check */
double _sqrt (a) 
double a; 
  {
  if (a < 0) longjmp (err_jump, E_NEGSQRT); 
  return sqrt (a); 
  }

/** asin with error check */
double _asin (a) 
double a; 
  {
  if (a < -1 || a > 1) longjmp (err_jump, E_TRGRNG); 
  if (angle_mode == AM_DEG)
    return RAD_TO_DEG * asin (a);
  else
    return asin (a); 
  }

/** asin with error check */
double _acos (a) 
double a; 
  {
  if (a < -1 || a > 1) longjmp (err_jump, E_TRGRNG); 
  if (angle_mode == AM_DEG)
    return RAD_TO_DEG * acos (a);
  else
    return acos (a); 
  }

/** atan2 with error check */
double _atan2 (a, b) 
double a, b; 
  {
  if (b == 0) longjmp (err_jump, E_DIVZ); 
  if (angle_mode == AM_DEG)
    return RAD_TO_DEG * atan2 (a, b);
  else
    return atan2 (a, b); 
  }

/** cos */
double _cos (a) 
double a; 
  {
  if (angle_mode == AM_DEG)
    a = a * DEG_TO_RAD;
  return cos (a); 
  }

/** atan2 with error check */
/** log with error check */
double _log (a) 
double a; 
  {
  if (a < 0) longjmp (err_jump, E_NEGLOG); 
  return log (a); 
  }

/** log10 with error check */
double _log10 (a) 
double a; 
  {
  if (a < 0) longjmp (err_jump, E_NEGLOG); 
  return log10 (a); 
  }

/** sin */
double _sin (a) 
double a; 
  {
  if (angle_mode == AM_DEG)
    a = a * DEG_TO_RAD;
  return sin (a); 
  }

/** tan */
double _tan (a) 
double a; 
  {
  if (angle_mode == AM_DEG)
    a = a * DEG_TO_RAD;
  return tan (a); 
  }



