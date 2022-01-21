/*===========================================================================

  kcalc-cpm

  compat.c

  Some functions missing or broken in the Aztec CP/M C library

  Copyright (c)2021 Kevin Boone, GPLv3.0

===========================================================================*/
#include "ctype.h"
#include "compat.h"
#include "stdio.h"

#ifndef CPM
/* Aztec C does not have these includes. There's no point replicated
 * the missing function prototypes, since the compiler doesn't support
 * function prototypes anyway. */
#include "string.h"
#include "stdlib.h"
#endif

/*
  memcpy
*/
void *_memcpy (dest, src, n)
void *dest;
CONST void *src;
int n;
  {
  register int i;
  for (i = 0; i < n; i++)
    ((char *)(dest))[i] = ((char *)(src))[i];
  return dest;
  }

/*
  memset
*/
void *_memset (s, c, n)
void *s;
int c;
int n;
  {
  register int i;
  for (i = 0; i < n; i++)
    ((char *)(s))[i] = c;
  return s;
  }

/*
  memmove
*/
void *_memmove (dest, src, n)
void *dest;
CONST void *src;
int n;
  {
  char *d = (char*)dest;
  char *s = (char*)src;
  if (s < d) 
    {
    s += n;
    d += n;
    while (n--)
      *--d = *--s;
    } 
  else
    {
    while (n--)
      *d++ = *s++;
    }

  return dest;
  }


/*
  strdup
*/
char *_strdup (s)
CONST char *s;
  {
  int l = strlen (s);
  char *ret = malloc (l + 1);
  strcpy (ret, s);
  return ret; 
  }

/*
  strchr
*/
char *_strchr (s, c)
CONST char *s;
int c;
  {
  for (; *s != '\0' && *s != c; ++s)
    ;
  return *s == c ? (char *) s : 0;
  }


/*
  _atof
  Can't use "atof" for the name, as a broken version already exists
  in the C library.
  I hope that using sscant as a substitute for atof() doesn't lead to
  an infinite recursion -- so far it seems OK. It wouldn't be hard to
  implement atof(), but if the funcionality already exists in the 
  C library, it would be wasteful to replicate it.
*/
double _atof (str)
CONST char *str;
  {
  double f = 99;
  sscanf (str, "%lf", &f);
  return f;
  }


/*
  _stdtod
  Can't use "strtod" for the name, as a broken version already exists
  in the C library.
*/
double _strtod (str, ptr)
char *str;
char **ptr;
  {
  char *p;
  if (ptr == (char **)0)
    return _atof (str);
  
  p = str;
  
  while (isspace (*p))
    ++p;

  if (*p == '+' || *p == '-')
    ++p;
  /* digits, with 0 or 1 periods in it.  */
  if (isdigit (*p) || *p == '.')
    {
      int got_dot = 0;
      while (isdigit (*p) || (!got_dot && *p == '.'))
        {
          if (*p == '.')
            got_dot = 1;
          ++p;
        }
      /* Exponent.  */
      if (*p == 'e' || *p == 'E')
        {
          int i;
          i = 1;
          if (p[i] == '+' || p[i] == '-')
            ++i;
          if (isdigit (p[i]))
            {
             while (isdigit (p[i]))
                ++i;
             *ptr = p + i;
             return _atof (str);
            }
        }
      *ptr = p;
      return _atof (str);
    }
  /* Didn't find any digits.  Doesn't look like a number.  */
  *ptr = str;
  return 0.0;
  }

/*
 strupr()
*/
void _strupr (s)
char *s;
  {
  while (*s != 0)
    {
    *s = toupper (*s);
    s++;
    }
  }

