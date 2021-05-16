/*===========================================================================

  tinyexpr.h

  Function prototypes in K&R style for the tinyexpr library, and additional
  constants used by the input line interpreter.

  Kevin Boone, May 2021

===========================================================================*/
#ifndef __TINYEXPR_H
#define __TINYEXPR_H

/* Syntax error */
#define E_SYNTAX  1
/* Div by zero */
#define E_DIVZ    2
/* Unknown identifier */
#define E_IDENT   3
/* Square root of a negative */
#define E_NEGSQRT 4
/* Log of a negative */
#define E_NEGLOG  5
/* Trig arg out of rng (e.g., acos -2)*/
#define E_TRGRNG  6
/* No identifier found where one expected */
#define E_NOIDENT 7
/* No expression found where one expected */
#define E_NOEXPR  8
/* No expression found where one expected */
#define E_MSYMS   9

/* TinyExpr variable/token types. */
#define TE_VARIABLE 0
#define TE_CONSTANT 1
#define TE_FUNC0 8 
#define TE_FUNC1 9 
#define TE_FUNC2 10
#define TE_FUNC3 11
#define TE_FUNC4 12
#define TE_FUNC5 13
#define TE_FUNC6 14
#define TE_FUNC7 15
#define TE_CLO0 16
#define TE_CLO1 17
#define TE_CLO2 18
#define TE_CLO3 19
#define TE_CLO4 20
#define TE_CLO5 21
#define TE_CLO6 22
#define TE_CLO7 23
#define TE_FLAG_PURE 32

#define TYPE_MASK(TYPE) ((TYPE)&0x0000001F)

typedef struct te_variable 
  {
  char *name;
  void *address;
  int type;
  void *context;
  double num; /* KB -- added to support variables created at runtime */
  } te_variable;

typedef int AngleMode;
extern AngleMode angle_mode;

#define AM_RAD 0
#define AM_DEG 1

typedef int BaseMode;
extern BaseMode base_mode;

#define BM_DEC  0
#define BM_HEX  1
#define BM_FRAC 2 

double te_interp ();

#endif

