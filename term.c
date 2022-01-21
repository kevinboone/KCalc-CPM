/*===========================================================================

  kcalc-cpm

  term.c
 
  Terminal handling functions

  Copyright (c)2021 Kevin Boone, GPL v3.0

===========================================================================*/

#include "stdio.h"
#include "ctype.h"
#include "term.h"
#include "config.h"

#ifdef CPM

/*===========================================================================

  term_get_rchar

  Get a raw char, without echoing

===========================================================================*/

int term_g_rchar ()
  {
  int c;
/*
  do
    {
    c = bdos (0x06, 0xFF);
    } while (c == 0);
  return c;
*/

  return bios (3, 0, 0); 
  }

/*===========================================================================

  term_get_ichar

  Get an interpreted char, without echoing

===========================================================================*/
int term_g_ichar ()
  {
  char c = term_g_rchar ();
  switch (c)
    {
    case I_INTR: return VK_INTR;
    case I_DESTBS: return VK_DESTBS;
    /* WordStar key bindings */
    case 'A' - '@': return VK_LWORD; 
    case 'S' - '@': return VK_LCHAR; 
    case 'D' - '@': return VK_RCHAR; 
    case 'F' - '@': return VK_RWORD; 
    case 'B' - '@': return VK_SOL; 
    }
  return c;
  }

/* TODO delete this */
void dump (buff)
char *buff;
  {
  int i;
  char c;
  for (i = 0; i < 10; i++)
    {
    c = buff[i];
    printf ("%02x %d '%c'\n", c, c, c);
    }
  }

/*===========================================================================

  term_g_line
 
  Read a line from the console, with editing

  Fills in the line, up to len characters. Caller should allocate one more
  character for the terminating zero. Returns zero if the program should
  continue, or non-zero on error or end-of-input.
  
===========================================================================*/
int term_g_line (line, len)
char *line;
int len;
  {
  int stop = 0;
  int pos = 0;
  line[0] = 0;
  while (!stop)
    {
    int c = term_g_ichar ();
    switch (c)
      {
      case VK_INTR: /* Interrupt */
        return 1;

      case VK_DESTBS: /* Destructive backspace */
        if (pos > 0)
	  {
          int i, l;
	  _memmove (line + pos - 1, line + pos, len - pos - 0);
	  pos--;
	  putchar (O_BS); 
          l = strlen (line);
          for (i = pos; i < l; i++) putchar (line[i]);
	  putchar (' '); 
          for (i = pos; i <= l; i++) putchar (O_BS);
	  }
	break;

      case VK_LCHAR: /* Left one char */ 
        if (pos > 0)
	  {
	  putchar (O_BS); 
	  pos--;
	  }
	break;

      case VK_LWORD: /* Left one word */ 
        if (pos == 1)
          {
          pos = 0;
          putchar (O_BS);
          }
        else
          {
          while (pos > 0 && isspace (line[(pos - 1)]))
            {
            pos--;
            putchar (O_BS);
            }
          while (pos > 0 && !isspace (line[pos - 1]))
            {
            pos--;
            putchar (O_BS);
            }
          }
        break;

      case VK_RWORD: /* Right one word */ 
        while (line[pos] != 0 && !isspace (line[pos]))
          {
          putchar (line[pos]);
          pos++;
          }
        while (line[pos] != 0 && isspace (line[pos]))
          {
          putchar (line[pos]);
          pos++;
          }
        break;

      case VK_SOL: /* Start of line */ 
        if (pos > 0)
          {
          int i;
          for (i = 0; i < pos; i++)
            putchar (O_BS);
          pos = 0;
          }
        else 
          {
          /* Wrap round to end of line */
          int i, l = strlen (line);
          for (i = pos; i < l; i++)
            putchar (line[i]);
          pos = l;
          }
        break;

      case VK_RCHAR: /* Right one char */ 
        {
        int l = strlen (line);
        if (pos < l)
	  {
          putchar (line[pos]);
          pos++;
	  }
        }
	break;

      case 10: case 13: 
	printf ("\r\n");
        stop = 1;
	break;

      default:
        if (pos < len - 1 && c >= 32)
	  {
          int i, l;
          char *p;
	  _memmove (line + pos + 1, line + pos, len - pos - 1);
          line[pos] = c;
          printf ("%s", line + pos);
          l = strlen (line + pos);
          for (i = 0; i < l - 1; i++) putchar (8);
          pos ++;
	  }
      }
    }
  fflush (stdout);
  return 0;
  }

#endif

#ifdef LINUX

int term_g_line (char *line, int len)
  {
  return fgets (line, len, stdin) == NULL;
  }

#endif

