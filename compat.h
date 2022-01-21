/*===========================================================================

  compat.h

  Conventional C functions that are not present in the Aztec 
  C library, or are present and broken. Note that these functions
  should _not_ be replaced by GlibC functions in Linux -- we want
  to test the implementations that will be used on CP/M, rather than
  replace them.

  In order for compile-time checking to work on Linux, we need to use
  proper function prototypes. However, the Aztec CP/M C compiler doesn't
  support function prototypes, so these must be ifdef'd out.

  Kevin Boone, May 2021

===========================================================================*/
#ifndef __COMPAT_H
#define __COMPAT_H

#ifdef CPM
#define CONST
#else
#define CONST const
#endif

#ifdef CPM
void *_memcpy ();
#else
void *_memcpy (void *dest, CONST void *src, int n);
#endif

#ifdef CPM
void *_memset ();
#else
void *_memset (void *s, int c, int n);
#endif

#ifdef CPM
void *_memmove ();
#else
void *_memmove (void *dest, CONST void *src, int n);
#endif

#ifdef CPM
char *_strdup ();
#else
char *_strdup (CONST char *str);
#endif

#ifdef CPM
void _strupr ();
#else
void _strupr (char *str);
#endif

#ifdef CPM
char *_strchr ();
#else
char *_strchr (CONST char *s, int c);
#endif

/** Convert hex string to decimal. This function actually _is_ in the
    Aztec C library, but it's broken */
#ifdef CPM
double _strtod ();
#else
double _strtod (char *nptr, char **endptr);
#endif

/** As above */
#ifdef CPM
double _atof ();
#else
double _atof (CONST char *nptr);
#endif

#endif
