/*===========================================================================

  compat.h

  Conventional C functions that are not present in the Aztec 
  C library, and a few other help functions that had to go somewhere.

  Kevin Boone, May 2021

===========================================================================*/
#ifndef __COMPAT_H
#define __COMPAT_H

void *memcpy ();
void *memset ();
void *memmove ();
char *strdup ();
char *strchr ();
double _strtod ();
/** Convert hex string to decimal */
double hstrtod ();

#endif

