/*===========================================================================

  term.h

  Terminal handling functions

  Kevin Boone, May 2021, GPL v3.0

===========================================================================*/
#ifndef __TERM_H
#define __TERM_H_

/* Interpreted key codes. To make it easier to support different key bindings
   in future, those parts of the proram that deal with character input work
   with "virtual" keycodes, indepdent of the physical terminal. However, 
   I presume that the standard ASCII alphanumeric code will always have
   the same meaning. */

/* Interrupt */
#define VK_INTR 1000

/* Destructive backspace */
#define VK_DESTBS 1001

/* Left one character */
#define VK_LCHAR 1002

/* Right one character */
#define VK_RCHAR 1003

/* Left one word */
#define VK_LWORD 1004

/* Right one word */
#define VK_RWORD 1005

/* Start of line */
#define VK_SOL 1006


/* Get a line from the console, with editing. 
   args: char *line, int len, ret: 0 if program should proceed. */
int term_g_line ();

/* Get a raw char from the console, without echoing.
    args: none. */
int term_g_rchar ();

/* Get an interpreted char from the console, without echoing.
    args: none. */
int term_g_ichar ();

#endif 

