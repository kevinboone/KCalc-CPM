/*===========================================================================

  stringutil.h

  Kevin Boone, May 2021

===========================================================================*/

#ifndef _STRINGUTIL_H
#define _STRINGUTIL_H

/** Convert a hex string to a double */
#ifdef CPM
double hstrtod ();
#else
double hstrtod (char *str, char **ptr);
#endif

/** Convert a hex digit to an integer number */
#ifdef CPM
int htod ();
#else
int htod (char digit);
#endif

/** Return TRUE if the supplied character is a hex digit */
#ifdef CPM
int ishexdigit ();
#else
int ishexdigit (char c);
#endif

#endif
