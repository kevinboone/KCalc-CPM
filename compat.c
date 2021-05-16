/*===========================================================================

  kcalc-cpm

  compat.c

  Copyright (c)2021 Kevin Boone, GPLv3.0

===========================================================================*/
#include "ctype.h"
#include "compat.h"

/*
  memcpy
*/
void *memcpy (dest, src, n)
void *dest;
void *src;
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
void *memset (s, c, n)
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
void *memmove (dest, src, n)
void *dest;
void *src;
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
  _atof
  Can't use "atof" for the name, as a broken version already exists
  in the C library.
  I hope that using sscant as a substitute for atof() doesn't lead to
  an infinite recursion -- so far it seems OK. It wouldn't be hard to
  implement atof(), but if the funcionality already exists in the 
  C library, it would be wasteful to replicate it.
*/
double _atof (str)
char *str;
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
  strdup
*/
char *strdup (s)
char *s;
  {
  int l = strlen (s);
  char *ret = malloc (l + 1);
  strcpy (ret, s);
  return ret; 
  }

/*
  strchr
*/
char *strchr (s, c)
char *s;
int c;
  {
  for (; *s != '\0' && *s != c; ++s)
    ;
  return *s == c ? (char *) s : 0;
  }

/*
  ishexdigit
*/
int ishexdigit (c)
char c;
  {
  if (c >= '0' && c <= '9') return 1;
  if (c >= 'a' && c <= 'f') return 1;
  if (c >= 'A' && c <= 'F') return 1;
  return 0;
  }

/* 
  htod
  hex digit to decimal
*/
static int htod (c)
int c;
  {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1; /* Should never happen */
  }

/*
  hstrtod
*/

double hstrtod (str, ptr)
char *str;
char **ptr;
  {
  double num = 0;
  char *p = str;
  *ptr = str;
  
  while (isspace (*p))
    ++p;

  if (!ishexdigit (*p)) return; /* Not a number */
  while (ishexdigit (*p))
    {
    num *= 16;
    num += htod (*p);
    p++;
    }

  *ptr = p;
  return num;
  }


