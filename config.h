/*===========================================================================

  config.h

  Various configuration parameters. 

  Kevin Boone, May 2021, GPL v3.0

===========================================================================*/
#ifndef __CONFIG_H
#define __CONFIG_H_

/* Character sent by terminal for an interrupt (usually ctrl+c) */
#define I_INTR  3

/* Character sent by the terminal for the "destuctive backspace" key 
 * This will usually be chr 8 on a real terminal, but may be 127 when
 * running the CP/M version under an emulator.
 */
#define I_DESTBS 8

/* Character to send to the terminal get non-destructive backspace */
#define O_BS 8

#endif


