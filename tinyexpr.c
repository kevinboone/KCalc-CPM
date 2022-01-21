/*===========================================================================

  kcalc-cpm

  tinyexpr.c

  This is a heavily modified version of the main part of
  TinyExpr, maintained by Lewis Van Winkle and distributed under the
  terms of a GPL-compatible licence. The changes have to do with 
  modifying the source so that it can be compiled with the kind of
  compilers that exist for CP/M. That means K&R-style function definitions,
  no constants, no enums, identifiers limited to 8 characters, etc.

  I've removed all the built-in math from this file, and created a new one 
  with additional error checking.

  My substantive changes (rather than just syntax changes) are marked
  with "KB"

  Modifications by Kevin Boone, May 2021

===========================================================================*/

#include "stdio.h"
#include "ctype.h"
#include "math.h"
#include "setjmp.h"
#include "tinyexpr.h"
#include "compat.h"
#include "strutil.h"
#ifdef LINUX
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#endif

#ifndef NAN
/* KB - The CP/M math library has no notion of "NaN", so use HUGE instead. */
#define NAN HUGE 
#endif

#define ARITY(TYPE) ( ((TYPE) & (TE_FUNC0 | TE_CLO0)) ? ((TYPE) & 0x00000007) : 0 )
#define IS_CLOSURE(TYPE) (((TYPE) & TE_CLO0) != 0)
#define IS_PURE(TYPE) (((TYPE) & TE_FLAG_PURE) != 0)

typedef double (*te_fun1)();
typedef double (*te_fun2)();

jmp_buf err_jump;
AngleMode angle_mode = AM_RAD;
BaseMode base_mode = BM_DEC;

typedef struct te_expr 
  {
  int type;
  double dvalue; 
  double *bound; 
  void *fvalue;
  void *parameters[1];
  } te_expr;

typedef struct state 
  {
  char *start;
  char *next;
  int type;
  double dvalue;
  double *bound;
  void *fvalue;
  void *context;

  te_variable *lookup;
  int lookup_len;
  } state;

#define TOK_NULL 24
#define TOK_ERROR 25
#define TOK_END 26
#define TOK_SEP 27
#define TOK_OPEN 28
#define TOK_CLOSE 29
#define TOK_NUMBER 30
#define TOK_VARIABLE 31
#define TOK_INFIX 32

static te_expr *power (); 
static te_expr *expr (); 
static te_expr *list (); 
static double te_eval (); 
static void te_free (); 

/* Implementation of missing trunc() function. */
double trunc (x)
double x;
  {
  if (x >= 0) return floor (x);
  return (ceil (x));
  }

static double add (a, b) double a; double b; {return a + b;}
static double sub (a, b) double a; double b; {return a - b;}
static double mul (a, b) double a; double b; {return a * b;}

/** KB -- divide with error check */
static double divide (a, b) 
double a; 
double b; 
  {
  if (b == 0) longjmp (err_jump, E_DIVZ); 
  return a / b;
  }


static double comma (a, b) double a; double b; {(void)a; return b;}

/** KB -- implement missing fmod */
#ifdef CPM
static double fmod (a, b) double a; double b; 
  {
  return a - (trunc (a/b) * b);
  }
#endif 

static double negate (a) double a; {return -a;}

/*
    Find an entry in the symbol table
*/
static te_variable *find_lookup (s, name, len) 
state *s;
char *name;
int len;
  {
  int iters;
  te_variable *var;
  if (!s->lookup) return 0;

  for (var = s->lookup, iters = s->lookup_len; iters; ++var, --iters) 
    {
    /* KB -- does this && op short-circuit? It would in a modern C, 
       and it needs to... */
    if (var->name && strncmp (name, var->name, len) == 0 
          && var->name[len] == '\0') 
      {
      return var;
      }
    }
  return 0;
  }

/*
    Get the next token and set the state accordingly 
*/
void next_token (s) 
  state *s;
  {
  s->type = TOK_NULL;
  do 
    {
    if (!*s->next)
      {
      s->type = TOK_END;
      return;
      }

    /* Try reading a number. */
    if (s->next[0] == '#') 
      {
      s->dvalue = hstrtod (s->next + 1, (char**)&s->next);
      s->type = TOK_NUMBER;
      }
    else if ((s->next[0] >= '0' && s->next[0] <= '9') || s->next[0] == '.') 
      {
      s->dvalue = _strtod (s->next, (char**)&s->next);
      s->type = TOK_NUMBER;
      }
    else 
      {
      /* Look for a variable or builtin function call. */
      if (isalpha(s->next[0])) 
        {
        te_variable *var;
        char *start;
        start = s->next;
        while (isalpha(s->next[0]) || isdigit(s->next[0]) 
	               || (s->next[0] == '_')) 
	  s->next++;
                
        var = find_lookup (s, start, s->next - start);

        if (!var) 
	  {
          s->type = TOK_ERROR;
	  longjmp (err_jump, E_IDENT);
          } 
        else 
	  {
          switch (TYPE_MASK(var->type))
            {
            case TE_VARIABLE:
              s->type = TOK_VARIABLE;
              s->bound = var->address;
              break;
            case TE_CLO0: case TE_CLO1: case TE_CLO2: 
	    case TE_CLO3: case TE_CLO4: case TE_CLO5: 
	    case TE_CLO6: case TE_CLO7:     
              s->context = var->context; /* Fall through */ 
            case TE_FUNC0: case TE_FUNC1: case TE_FUNC2: 
	    case TE_FUNC3: case TE_FUNC4: case TE_FUNC5: 
	    case TE_FUNC6: case TE_FUNC7:   
              s->type = var->type;
              s->fvalue = var->address;
              break;
            }
          }
        } 
      else 
        {
        /* Look for an operator or special character. */
        switch (s->next++[0]) 
          {
          case '+': s->type = TOK_INFIX; s->fvalue = add; break;
          case '-': s->type = TOK_INFIX; s->fvalue = sub; break;
          case '*': s->type = TOK_INFIX; s->fvalue = mul; break;
          case '/': s->type = TOK_INFIX; s->fvalue = divide; break;
          case '^': s->type = TOK_INFIX; s->fvalue = pow; break;
          case '%': s->type = TOK_INFIX; s->fvalue = fmod; break;
          case '(': s->type = TOK_OPEN; break;
          case ')': s->type = TOK_CLOSE; break;
          case ',': s->type = TOK_SEP; break;
          case ' ': case '\t': case '\n': case '\r': break;
          default: s->type = TOK_ERROR; break;
          }
        }
      }
    } while (s->type == TOK_NULL);
  }

/*
    Free the parameters assigned to an expression, when it represents a
    function call with arguments. 
*/
void te_fp (n) 
te_expr *n;
  {
  if (!n) return;
  switch (TYPE_MASK(n->type)) 
    {
    case TE_FUNC7: case TE_CLO7: te_free (n->parameters[6]);     /* Falls through. */
    case TE_FUNC6: case TE_CLO6: te_free (n->parameters[5]);     /* Falls through. */
    case TE_FUNC5: case TE_CLO5: te_free (n->parameters[4]);     /* Falls through. */
    case TE_FUNC4: case TE_CLO4: te_free (n->parameters[3]);     /* Falls through. */
    case TE_FUNC3: case TE_CLO3: te_free (n->parameters[2]);     /* Falls through. */
    case TE_FUNC2: case TE_CLO2: te_free (n->parameters[1]);     /* Falls through. */
    case TE_FUNC1: case TE_CLO1: te_free (n->parameters[0]);
    }
 }

/*
    Free memory used to represent an expression.
*/
void te_free(n) 
te_expr *n;
  {
  if (!n) return;
  te_fp(n);
  free(n);
  }

/*
    Where possible, evalate those parts of an expression whose
    values are already known. (KB -- this facility has no particular
    benefit in KCalc-CPM)
*/
static void optimize (n) 
te_expr *n;
  {
  /* Evaluates as much as possible. */
  if (n->type == TE_CONSTANT) return;
  if (n->type == TE_VARIABLE) return;

  /* Only optimize out functions flagged as pure. */
  if (IS_PURE(n->type)) 
    {
    int arity = ARITY(n->type);
    int known = 1;
    int i;
    for (i = 0; i < arity; ++i) 
      {
      optimize (n->parameters[i]);
      if (((te_expr*)(n->parameters[i]))->type != TE_CONSTANT) 
        {
        known = 0;
        }
      }
    if (known) 
      {
      double value = te_eval (n);
      te_fp(n);
      n->type = TE_CONSTANT;
      n->dvalue = value;
      }
    }
  }

/*
    Allocate memory for a new expression object, with a variable number
    of paramters.
*/
static te_expr *new_expr (type, parameters) 
int type; 
te_expr *parameters[];
  {
  int arity = ARITY (type);
  int psize = sizeof(void*) * arity;
  int size = (sizeof(te_expr) - sizeof(void*)) + psize + (IS_CLOSURE (type) ? sizeof(void*) : 0);
  te_expr *ret = malloc(size);
  _memset(ret, 0, size);
  if (arity && parameters) 
    {
    _memcpy (ret->parameters, parameters, psize);
    }
  ret->type = type;
  ret->bound = 0;
  return ret;
  }

/*
    Parse current token as a terminal symbol (constant, function call...)
*/
static te_expr *base (s) 
state *s;
  /* <base>      =    <constant> | <variable> | <function-0> {"(" ")"} | <function-1> <power> | <function-X> "(" <expr> {"," <expr>} ")" | "(" <list> ")" */
  {
  te_expr *ret;
  int arity;

  switch (TYPE_MASK (s->type)) 
    {
    case TOK_NUMBER:
      ret = new_expr (TE_CONSTANT, 0);
      ret->dvalue = s->dvalue;
      next_token(s);
      break;

    case TOK_VARIABLE:
      ret = new_expr (TE_VARIABLE, 0);
      ret->bound = s->bound;
      next_token(s);
      break;

    case TE_FUNC0:
    case TE_CLO0:
      ret = new_expr(s->type, 0);
      ret->fvalue = s->fvalue;
      if (IS_CLOSURE(s->type)) ret->parameters[0] = s->context;
        next_token(s);
      if (s->type == TOK_OPEN) 
	{
        next_token(s);
        if (s->type != TOK_CLOSE) 
	  {
          s->type = TOK_ERROR;
          } 
	else 
	  {
          next_token(s);
          }
        }
      break;

    case TE_FUNC1:
    case TE_CLO1:
      ret = new_expr(s->type, 0);
      ret->fvalue = s->fvalue;
      if (IS_CLOSURE(s->type)) ret->parameters[1] = s->context;
      next_token(s);
      ret->parameters[0] = power(s);
      break;

    case TE_FUNC2: case TE_FUNC3: case TE_FUNC4:
    case TE_FUNC5: case TE_FUNC6: case TE_FUNC7:
    case TE_CLO2: case TE_CLO3: case TE_CLO4:
    case TE_CLO5: case TE_CLO6: case TE_CLO7:
      arity = ARITY(s->type);

      ret = new_expr(s->type, 0);
      ret->fvalue = s->fvalue;
      if (IS_CLOSURE(s->type)) ret->parameters[arity] = s->context;
      next_token(s);

      if (s->type != TOK_OPEN) 
	{
        s->type = TOK_ERROR;
        } 
      else 
	{
        int i;
        for (i = 0; i < arity; i++) 
	  {
          next_token(s);
          ret->parameters[i] = expr(s);
          if(s->type != TOK_SEP) 
	    {
            break;
            }
          }
        if(s->type != TOK_CLOSE || i != arity - 1) 
	  {
          s->type = TOK_ERROR;
          } 
	else 
	  {
          next_token(s);
          }
        }
      break;

    case TOK_OPEN:
      next_token(s);
      ret = list(s);
      if (s->type != TOK_CLOSE) 
        {
        s->type = TOK_ERROR;
        } 
      else 
	{
        next_token(s);
        }
      break;

    default:
      ret = new_expr (0, 0);
      s->type = TOK_ERROR;
      ret->dvalue = NAN;
      break;
    }
  return ret;
  }

/*
  Grammar rule:
  <power> = {("-" | "+")} <base> 
*/
static te_expr *power (s) 
state *s;
  {
  te_expr *ret;
  int sign = 1;

  while (s->type == TOK_INFIX && (s->fvalue == add || s->fvalue == sub)) 
    {
    if (s->fvalue == sub) sign = -sign;
    next_token (s);
    }

  if (sign == 1) 
    {
    ret = base (s);
    } 
  else 
    {
    /* ??? */
    te_expr *a[1];
    a[0] = base (s);
    ret = new_expr (TE_FUNC1 | TE_FLAG_PURE, a);
    ret->fvalue = negate;
    }
  return ret;
  }

/*
  Grammar rule:
  <factor> = <power> {"^" <power>} 
*/
static te_expr *factor(s) 
state *s;
  {
  te_expr *ret = power(s);

  while (s->type == TOK_INFIX && (s->fvalue == pow)) 
    {
    /* ??? */
    te_expr *a[2];
    te_fun2 t = s->fvalue;
    next_token(s);
    a[0] = ret;
    a[1] = power (s);
    ret = new_expr (TE_FUNC2 | TE_FLAG_PURE, a);
    ret->fvalue = t;
    }

  return ret;
  }

/*
  Grammar rule:
  <term> = <factor> {("*" | "/" | "%") <factor>} 
*/
static te_expr *term (s) 
state *s;
  {
  te_expr *ret = factor (s);

  while (s->type == TOK_INFIX && (s->fvalue == mul 
    || s->fvalue == divide || s->fvalue == fmod)) 
    {
    te_expr *a[2];
    te_fun2 t = s->fvalue;
    next_token(s);
    a[0] = ret;
    a[1] = factor (s);
    ret = new_expr (TE_FUNC2 | TE_FLAG_PURE, a);
    ret->fvalue = t;
    }

    return ret;
}

/*
  Grammar rule:
   <expr> = <term> {("+" | "-") <term>} 
*/
static te_expr *expr (s) 
state *s;
  {
  te_expr *ret = term (s);

  while (s->type == TOK_INFIX && (s->fvalue == add || s->fvalue == sub)) 
    {
    te_expr *a[2];
    te_fun2 t = s->fvalue;
    next_token (s);
    a[0] = ret;
    a[1] = term(s);
    ret = new_expr (TE_FUNC2 | TE_FLAG_PURE, a);
    ret->fvalue = t;
    }

  return ret;
  }

/*
  Grammar rule:
  <list> = <expr> {"," <expr>} 
*/
static te_expr *list(s) 
state *s;
  {
  te_expr *ret = expr (s);

  while (s->type == TOK_SEP) 
    {
    te_expr *a[2];
    next_token(s);
    a[0] = ret;
    a[1] = expr (s);
    ret = new_expr (TE_FUNC2 | TE_FLAG_PURE, a);
    ret->fvalue = comma;
    }

  return ret;
  }

/*
   Build the syntax tree.
*/
te_expr *te_compile (expression, variables, var_count, error) 
char *expression;
te_variable *variables;
int var_count;
int *error;
  {
  state s;
  te_expr *root; 
  s.start = s.next = expression;
  s.lookup = variables;
  s.lookup_len = var_count;

  next_token(&s);
  root = list (&s);

  if (s.type != TOK_END) 
    {
    te_free(root);
    if (error) 
      {
      *error = (s.next - s.start);
      if (*error == 0) *error = 1;
      }
    return 0;
    } 
  else 
    {
    optimize (root);
    if (error) *error = 0;
    return root;
    }
  }

/*
   Evaluate a specific node in the syntax tree. 
*/

#define M(e) te_eval (n->parameters[e])

double te_eval (n) 
te_expr *n;
  {
  if (!n) return NAN; /* Should not happen */

  switch (TYPE_MASK(n->type)) 
    {
    /* KB -- I've removed some of the logic from the original tineyexpr
       here, that will not be used by KCalc-CPM. In particular, not
       function has more than two arguments. */

    case TE_CONSTANT: return n->dvalue;
    case TE_VARIABLE: return *n->bound;

    case TE_FUNC1:
      return ((te_fun1)n->fvalue) (M(0));

    case TE_FUNC2:
      return ((te_fun2)n->fvalue) (M(0), M(1));

    default: return 42;
    }
  return 99;
  }

/*
    Parse the input express to a syntax tree, and evaluate it to
    a number. 
    KB -- I've added some error return codes here, so the caller
    can format a slightly better error message. On exit, rt_error is
    set if an error occured either when parsing or evaluating the
    expression. 
*/
double te_interp (expression, error_pos, rt_error, vars, nvars) 
char *expression;
int *error_pos;
te_variable *vars[];
int nvars;
int *rt_error;
  {
  double ret;
  te_expr *n;
  int rt_err = setjmp (err_jump);
 
  /* KB -- This is where we end up if any math function raises an exception
     using longjmp(), as well as during normal execution. The return 
     value from setjmp() allows us to separate these two cases. */

  if (rt_err != 0)
    {
    *error_pos = -1;
    *rt_error = rt_err;
    return NAN;
    }
  *error_pos = 0;
  *rt_error = 0;
  n = te_compile (expression, vars, nvars, error_pos);
  if (n) 
    {
    ret = te_eval(n);
    te_free(n);
    } 
  else 
    {
    ret = NAN;
    *rt_error = E_SYNTAX;
    }
  return ret;
  }

