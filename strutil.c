/*===========================================================================

  strutil.c

  Kevin Boone, Jan 2022 

===========================================================================*/
#include "ctype.h"

#ifndef CPM
#include <ctype.h>
#endif

/* 
  htod
  hex digit to decimal
*/
int htod (c)
int c;
  {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1; /* Should never happen */
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

  if (!ishexdigit (*p)) return 0; /* Not a number */
  while (ishexdigit (*p))
    {
    num *= 16;
    num += htod (*p);
    p++;
    }

  *ptr = p;
  return num;
  }


