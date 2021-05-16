/*===========================================================================

  kcalc-cpm

  main.c

  Main loop, interface to parser, error handling.

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "ctype.h"
#include "tinyexpr.h"
#include "ctype.h"
#include "math.h"
#include "funcs.h"
#include "term.h"
#include "config.h"

#define BANNER1 "kcalc-cpm version 0.1a, May 2021.\r\n"
#define BANNER2 "Enter \"help\" for instructions, \"quit\" to exit.\r\n"

/** The main symbol table */
#define SYMTAB_MAX 30
static te_variable symtab[SYMTAB_MAX];
/* Number of entries in the symbol table. Note that nsyms _includes_
   symtab entries that are currently empty.  */
int nsyms = 0; 

double ans = 0; /* Last answer */

void kc_set_num (); /* Fwd ref */

/*===========================================================================

  kc_toupper

  Convert a string to upper case

===========================================================================*/
void kc_toupper (s)
char *s;
  {
  while (*s)
    {
    *s = toupper (*s);
    s++;
    }
  }

/*===========================================================================

  kc_strerror

  Get a textual represetation of an error code

===========================================================================*/
char *kc_strerror (code)
int code;
  {
  if (code == E_SYNTAX) return "Syntax error";
  if (code == E_DIVZ) return "Division by zero";
  if (code == E_IDENT) return "Unknown identifier";
  if (code == E_NEGSQRT) return "Square root of negative number";
  if (code == E_NEGLOG) return "Logarithm of negative number";
  if (code == E_TRGRNG) return "Trig argument out of range";
  if (code == E_NOIDENT) return "Missing identifier";
  if (code == E_NOEXPR) return "Missing expression";
  if (code == E_MSYMS) return "Symbol table full";
  return "Unknown error";
  }

/*===========================================================================

  kc_keys

  Shows key bindings 

===========================================================================*/
void kc_keys ()
  {
  printf ("ctrl+a          left one word\r\n");
  printf ("ctrl+b          start of line\r\n");
  printf ("ctrl+b, ctrl-b  end of line\r\n");
  printf ("ctrl+c          quit\r\n");
  printf ("ctrl+d          right one character\r\n");
  printf ("ctrl+f          right one word\r\n");
  printf ("ctrl+h/BS       erase character left\r\n");
  printf ("ctrl+s          left one character\r\n");
  }

/*===========================================================================

  kc_status

  Show current settings

===========================================================================*/
void kc_status ()
  {
  if (angle_mode == AM_DEG)
    printf ("Angle mode is degrees. use RAD to set it to radians.\r\n");
  else
    printf ("Angle mode is radians. use DEG to set it to degrees.\r\n");
  if (base_mode == BM_DEC)
    printf ("Output base is decimal. use HEX to set it to hexadecimal.\r\n");
  else
    printf ("Output base is hexadecimal. use DEC to set it to decimal.\r\n");
  }

/*===========================================================================

  kc_help

  Show brief help text

===========================================================================*/
void kc_help ()
  {
  printf (BANNER1);
  printf 
("Enter mathematical expressions at the prompt (or on the command line).\r\n");
  printf 
("Enter \"list\" for a list of functions, constants, and commands.\r\n");
  printf 
("Enter \"keys\" for information about line editing keys.\r\n");
  printf 
("Enter \"status\" for current settings.\r\n");
  printf 
("For more information: http://kevinboone.me/kcalc-cpm.html.\r\n");
  }


/*===========================================================================

  kc_do_list

  Lists functions and variables

===========================================================================*/
void kc_do_list ()
  {
  register int i;

  printf ("Constants/variables:\r\n");
  for (i = 0; i < nsyms; i++)
    {
    te_variable *sym = &symtab[i];
    if (TYPE_MASK (sym->type) == TE_CONSTANT 
             || TYPE_MASK (sym->type) == TE_VARIABLE)
      if (sym->name) printf ("%s\r\n", sym->name);
    }
  printf ("\r\n");

  printf ("Functions:\r\n");
  for (i = 0; i < nsyms; i++)
    {
    te_variable *sym = &symtab[i];
    if (sym->name)
      {
      if (TYPE_MASK (sym->type) == TE_FUNC1 
          || TYPE_MASK (sym->type) == TE_FUNC2)
        {
        if (TYPE_MASK (sym->type) == TE_FUNC1)
          printf ("%s(x)\r\n", sym->name);
        else
          printf ("%s(x,y)\r\n", sym->name);
        }
      }
    }
  printf ("\r\n");
  printf ("Commands:\r\n");
  printf ("DEC\r\n");
  printf ("DEG\r\n");
  printf ("HEX\r\n");
  printf ("LIST\r\n");
  printf ("HELP\r\n");
  printf ("KEYS\r\n");
  printf ("QUIT\r\n");
  printf ("RAD\r\n");
  }

/*===========================================================================

  kc_do_cmd

  Returns non-zero if the argument was processed as a command, whether it
  succeeded or not.

===========================================================================*/
int kc_do_cmd (line)
char *line;
  {
  if (strncmp (line, "LIST", 4) == 0)
    {
    kc_do_list (); return 1;
    }
  else if (strncmp (line, "DEG", 3) == 0)
    {
    angle_mode = AM_DEG; return 1;
    }
  else if (strncmp (line, "HELP", 4) == 0)
    {
    kc_help (); return 1;
    }
  else if (strncmp (line, "STATUS", 6) == 0)
    {
    kc_status (); return 1;
    }
  else if (strncmp (line, "KEYS", 4) == 0)
    {
    kc_keys (); return 1;
    }
  else if (strncmp (line, "RAD", 3) == 0)
    {
    angle_mode = AM_RAD; return 1;
    }
  else if (strncmp (line, "DEC", 3) == 0)
    {
    base_mode = BM_DEC; return 1;
    }
  else if (strncmp (line, "HEX", 3) == 0)
    {
    base_mode = BM_HEX; return 1;
    }
  /* Return 0 if we didn't recongize the line as a command. We don't 
     return any error code from this function, because there aren't any
     errors that can be raised. */
  return 0;
  }

/*===========================================================================

  kc_eval

  Evaluate the expression and return a number. Set the error indicator
  to true, and return HUGE if the evaluation fails. This function
  displays an error message on failure, so callers should not do so.  

===========================================================================*/
double kc_eval (expr, error, vars, nvars)
char *expr;
te_variable *vars[];
int nvars;
int *error;
  {
  double ret = HUGE;
  double result;
  int error_pos = 0;
  int rt_error = 0;
  *error = 1;

  result = te_interp (expr, &error_pos, &rt_error, vars, nvars);

  if (rt_error == 0)
    {
    ret = result;
    *error = 0;
    }
  else
    {
    printf ("%s ", kc_strerror (rt_error));
    if (error_pos > 0)
      {
      if (error_pos >= strlen (expr))
	printf ("at end of line"); 
      else
	printf ("at position %d", error_pos); /* TODO -- nicer message */
      }
    printf ("\n");
    }

  return ret;
  }

/*===========================================================================

  kc_trim_right
 
  Trim whitespace on the right.

===========================================================================*/
void kc_trim_right (s)
char *s;
  {
  int l = strlen (s);
  if (l == 0) return;
  l--;
  while (l >= 0 && isspace (s[l]))
    s[l--] = 0; 
  }

/*===========================================================================

  kc_do_assign

  Parse the line as an assignment. If it can be parsed, return 1, whether
  it succeeds or not. Display error if it fails.
 
  This is all very ugly -- this assignment parsing ought to be integrated
  into the main expression parser.

===========================================================================*/
int kc_do_assign (line, vars, nvars) 
char *line;
te_variable *vars[];
int nvars;
  {
  char *eqp = strchr (line, '=');
  if (eqp)
    {
    /* We are modifying the caller's string here. Check whether that's OK */
    char *sval;
    *eqp = 0; 
  
    kc_trim_right (line);
    sval = eqp + 1;
    while (*sval && isspace (*sval))
      sval++;

    if (line[0])
      {
      if (sval[0])
        {
        int error = 0;
        double result = kc_eval (sval, &error, vars, nvars);
        /* kc_eval will already have displayed any error */
        if (!error)
	  {
          kc_set_num (line, result);
	  }
        }
      else
        {
        printf ("%s\r\n", kc_strerror (E_NOEXPR));
        }
      }
    else 
      {
      printf ("%s\r\n", kc_strerror (E_NOIDENT));
      }

    return 1;
    }
  else
    return 0;
  }


/*===========================================================================

  kc_fmt

  Format a number for display

===========================================================================*/
void kc_fmt (num)
double num;
  {
  /* TODO */
  if (base_mode == BM_HEX)
    {
    if (num < 0)
      {
      printf ("-");
      num = -num;
      }
    printf ("#%lx\n", (long)num);
    }
  else
    printf ("%.9lg\n", num);
  }

/*===========================================================================

  kc_do_line

  Process one line, which might be a command, an expression, or an
  assignment. No error return -- messages are displayed internally.

===========================================================================*/
void kc_do_expr (expr, vars, nvars)
char *expr;
te_variable *vars;
int nvars;
  {
  if (expr[0] == 0 || expr[0] == 10 || expr[0] == 13) return;

  if (kc_do_cmd (expr) == 0)
    {
    if (kc_do_assign (expr, vars, nvars) == 0)
      {  
      int error = 0;
      double result = kc_eval (expr, &error, vars, nvars);
      if (!error)
	{
	/* Format properly, strip trailing zeros after the point, etc */
        kc_fmt (result);
	ans = result;
	}
      }
    }
  }

/*===========================================================================

  kc_do_repl

  This is the main interactive loop

===========================================================================*/
void kc_do_repl ()
  {
  char line [128];
  int done = 0;
  printf (BANNER1);
  printf (BANNER2);
  printf ("\r\n");
  while (!done)
    {
    printf ("kcalc> ");
    fflush (stdout);
    if (term_g_ln (line, sizeof (line) - 1) == 0)
      {
      kc_toupper (line);
      if (strncmp (line, "QUIT", 4) == 0) done = 1;
      }
    else
      done = 1;
    if (!done)
      {
      printf ("\r"); /* Need this with a terminal, if CR does not imply LF */
      kc_do_expr (line, symtab, nsyms); 
      printf ("\r\n");
      fflush (stdout);
      }
    }
  }


/*===========================================================================

  kc_add_num

  Add a number variable to the symbol table, if there is room. Returns 
  an error code if there is not.

===========================================================================*/
int kc_add_num (name, value)
char *name;
double value;
  {
  int i = nsyms;
  if (i >= SYMTAB_MAX - 1) return E_MSYMS;
  symtab[i].name = strdup (name);
  symtab[i].type = TE_VARIABLE;
  symtab[i].num = value;
  symtab[i].address = &(symtab[i].num);
  nsyms++;
  return 0;
  }

/*===========================================================================

  kc_add_var

  Add a variable with global scope to the symtab. Only used for "ans"
  at present.

  At present, this is only called at startup, so no need to check errors.

===========================================================================*/
void kc_add_var (name, address)
char *name;
void *address;
  {
  int i = nsyms;
  symtab[i].name = strdup (name);
  symtab[i].type = TE_VARIABLE;
  symtab[i].address = address;
  nsyms++;
  /** TODO check overflow */
  }

/*===========================================================================

  kc_add_1func

  Add a one-arg function to the symtab

  At present, this is only called at startup, so no need to check errors.

===========================================================================*/
void kc_add_1func (name, address)
char *name;
void *address;
  {
  int i = nsyms;
  symtab[i].name = strdup (name);
  symtab[i].type = TE_FUNC1 | TE_FLAG_PURE;
  symtab[i].address = address;
  /** TODO check overflow */
  nsyms++;
  }

/*===========================================================================

  kc_add_2func

  Add a two-arg function to the symtab

  At present, this is only called at startup, so no need to check errors.

===========================================================================*/
void kc_add_2func (name, address)
char *name;
void *address;
  {
  int i = nsyms;
  symtab[i].name = strdup (name);
  symtab[i].type = TE_FUNC2 | TE_FLAG_PURE;
  symtab[i].address = address;
  /** TODO check overflow */
  nsyms++;
  }

/*===========================================================================

  kc_find_sym 

===========================================================================*/
static te_variable *kc_find_sym (name)
char *name;
  {
  int i;
  for (i = 0; i < nsyms; i++)
    {
    te_variable *sym = &symtab[i];
    if (sym->name)
      {
      if (strcmp (sym->name, name) == 0) return sym;
      }
    }
  return 0;
  }


/*===========================================================================

  kc_clear_syms 

  Clean up dynamically-allocated memory in symbol table. Not strictly
  necessary, but it's inelegant not to, and defeats memory-leaker
  checkers.

===========================================================================*/
void kc_clear_syms ()
  {
  int i;
  for (i = 0; i < nsyms; i++)
    {
    te_variable *sym = &symtab[i];
    if (sym->name) free (sym->name);
    sym->name = 0;
    }
  }

/*===========================================================================

  te_empty_var 

  Find a free variable slot in the symtab, if there is one

===========================================================================*/
te_variable *kc_empty_var (name)
char *name;
  {
  int i;
  for (i = 0; i < nsyms; i++)
    {
    te_variable *sym = &symtab[i];
    if (!sym->name) return sym;
    }
  return 0;
  }

/*===========================================================================

  kc_set_num

  Set a number variable to a valuue, making space for it if
  necessary. If the variable already exists, it gets overwritten.

===========================================================================*/
void kc_set_num (name, value)
char *name;
double value;
  {
  te_variable *te = kc_find_sym (name);
  if (te)
    {
    if (TYPE_MASK (te->type) == TE_VARIABLE)
      {
      te->num = value;
      }
    }
  else
    {
    te_variable *te = kc_empty_var (name);
    if (te)
      {
      te->name = strdup (name);
      te->type = TE_VARIABLE;
      te->address = &(te->num);
      te->num = value;
      }
    else
      {
      int err = kc_add_num (name, value);
      if (err) printf ("%s\r\n", kc_strerror (err));
      }
    }
  }


/*===========================================================================

  main

===========================================================================*/
int main (argc, argv)
int argc;
char **argv;
  {
  te_variable *dummy;
  int i;
  char line [128];
  kc_add_num ("PI", CONST_PI);
  kc_add_num ("E", CONST_E);
  kc_add_var ("ANS", &ans);
  kc_add_1func ("ABS", fabs);
  kc_add_1func ("ACOS", _acos);
  kc_add_1func ("ASIN", _asin);
  kc_add_1func ("ATAN", _atan); 
  kc_add_2func ("ATAN2", _atan2); 
  kc_add_1func ("CEIL", ceil); 
  kc_add_1func ("COS", _cos); 
  kc_add_1func ("COSH", cosh); 
  kc_add_1func ("EXP", exp); 
  kc_add_1func ("FLOOR", floor); 
  kc_add_1func ("LOG", _log); 
  kc_add_1func ("LOG10", _log10); 
  kc_add_2func ("POW", pow); 
  /*kc_add_1func ("FAC", fac);*/ 
  kc_add_1func ("SIN", _sin); 
  kc_add_1func ("SINH", sinh); 
  kc_add_1func ("SQRT", _sqrt);
  kc_add_1func ("TAN", _tan); 
  kc_add_1func ("TANH", tanh); 

  line [0] = 0;
  for (i = 1; i < argc; i++)
    {
    char *arg = argv[i];
    int l = strlen (arg);
    int ll = strlen (line);
    if (ll + l + 2 < sizeof (line))
      {
      strcat (line, arg);
      strcat (line, " ");
      }
    }

  if (line[0])
    kc_do_expr (line, symtab, nsyms);
  else
    kc_do_repl (); 

  kc_clear_syms ();
  }


