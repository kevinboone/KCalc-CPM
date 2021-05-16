#KCalc CPM

Version 0.1a, May 2021

## What is this?

KCalc-CPM is an all-new, scientific calculator utility for the CP/M 2.2
platform. I wrote it largely as an educational exercise, to run on the "Z80
Playground" single-board computer, but it runs on other CP/M implementations as
well. I would have expected utilities of this sort to be widely available
for CP/M, but I could not find any that I liked, and that had full
floating-point and scientific notation support. 

KCalc-CPM has an interactive mode with rudimentary, WordStar-like line editing;
it can also be invoked directly from the command line:

    A> kcalc sin (pi/4)
    0.70710678     

The floating point math library the program uses provides about 
11-12 signigicant figures of precision, and results are displayed to
nine figures.

KCalc-CPM follows proper arithmetic rules of precence; brackets can
be used to group terms in the conventional way.

A number of logarithmic and trig functions are provided -- type
"list" at the prompt to see them all.

The usual math operators are supported; to calculate a power use
`^`, e.g., `2^16`. The modulus (remainder) operator is 
`%`, e.g., `7%3`.

KCalc-CPM supports decimal and hexadecimal entry and display.

KCalc-CPM is written in an archaic form of the C programming
language, and can be compiled on a CPM machine, if you have the
patience.

## Installation

Just copy `kcalc.com` to the A drive, or any convenient drive. 

## Running 

Enter `kcalc` at the CP/M prompt; use a drive letter 
(e.g., `i:kcalc`) if the program is not installed on a drive that
CP/M automatically searches. You can enter an expression directly
on the command line; if you don't, the program will enter interactive
mode. To leave interactive mode, enter "quit", or just hit 
ctrl+C.

## Line editor

KCalc-CPM supports basic line editing, with WordStar-like key bindings.
To get a list of key assignments, type "keys" at the prompt. 

## Numbers

KCalc-CPM accepts decimal numbers in ordinary decimal notation, or in 
scientific notation (e.g., `1.2E10`). Hexadecimal numbers can be entered
starting with `#`. So `#20` is 32 decimal. To enter a negative hex number,
put the minus sign first: `-#20`. Hexadecimal numbers can't be fractional,
or use scientific notation. Both formats are mathematically possible, but
I didn't see a need to add extra code for a feature that is hardly
likely to be used. 

Results are displayed in decimal or hexadecimal: use `dec` or
`hex` at the prompt to select which is used.

## Variables

You can define new variables like this:

    twopi = 2 * pi

Variable names can be up to 126 characters long, but there's little reason
for them to be. Variables can freely
be used in later expressions. Variable names are not case-sensitive.
By default, up to 10 variables can be used in a particular session.
Variables can't be deleted to reduce storage -- they take up a fixed
amount of memory determined at compile time. 

You won't be able to define a variable with the same name, or even starting
with the same letters, as a command -- the whole line will be treated
as a command.

## Notes

All function and variable names are case-insensitive -- they have to be
because that's how the CP/M CCP presents them to us if they are used on
the command line.

All angles are in radians by default; to use degrees type `deg` at the prompt. 

`ans` is a variable that denotes the last result calculated:

    A> kcalc
    kcalc> pi/2
    1.57079633
  
    kcalc> sin (ans)    
    1.000000000

There's no universal agreement on whether the value of the modulus
operator applied to a negative number is negative or positive. 
KCalc-CPM gives a negative result in such situations.

The `log` function is the _natural_ (base-e) logarithm -- use `log10`
for common (base-10) logarith. The `exp` function is the base-e
antilogarithm, that is, e to the power x. There is no common antilogarithm
function -- just do "10^x".

KCalc-CPM uses about 40kB of RAM -- 36kB for the program, and the rest
for its working storage.

## Building on a CP/M machine

KCalc-CPM is designed to be capable of being built under CP/M, using the
Aztec C compiler. The CP/M version of Aztec can be obtained here

https://www.aztecmuseum.ca/compilers.htm#cpm 

It's free of charge but not, so far as I know, open source. It's
easiest to build if the C compiler files and the source for
KCalc-CPM are on the same disk. In that case, you can just 
`submit` the file `kcalc.sub`. But be patient -- it will take ten
minutes or so on the Z80 Playground. 

I build KCalc-CPM using a Linux emulation set-up, because life is
short. It isn't documented here, but I'd be happy to describe it, 
if anybody is interested.

## Limitations

The maximum line length in the editor is 127 characters, which is also the
maximum length of the CP/M command line. It's at least possible that trying to
enter a longer line that this will cause a crash, because the line editor
doesn't check as diligently as it should for buffer overflows. How well a line
that is longer than the screen is handled depends on the terminal -- for
example, some terminals won't backspace from the beginning of one line to the
end of the previous one, and there's little that KCalc can do about that.

There is, at present, no way to define new functions, or to read definitions
from a file, or to log results to a file.

KCalc-CPM uses a direct BIOS call to read characters from the console. 
That's a bit naughty, but there are technical reasons why this works better
than using the "proper" BDOS calls -- not on a real Z80 board necessarily, but
on an emulator.

KCalc-CPM can only display hexadecimal numbers up to #FFFFFFFF. 
Attempting to display a number larger than this in HEX mode will lead
to odd, completely bogus, results.

Hexadecimal arithmetic is done in floating-point format just as
decimal arithmetic is. Hex calculations won't overflow and change
sign, as they would if carried out with integers in CPU registers.
There are other CP/M utilities for doing integer calculations of
this kind.

I test KCalc-CPM using the Minicom terminal emulator, with its 
default settings. I'm not sure how well it works with other terminals,
or what set-up might be required. The program outputs a CR-LF sequence
to indicate the end of a line. It sends character 8 to indicate a 
cursor-back operation, which it expects to be non-destructive on
the terminal. However, if it _receives_ character 8, it is 
treated as a delete ("rubout") character, not a cursor movement.
There is no way to change these settings at runtime. 

KCalc-CPM uses WordStar-style line editing keys not just for
authenticity, but because handling ANSI arrow keys is fiddly, 
and would require a lot of additional code. 

The "word-left" keystroke, ctrl+A is also, by default,
the "menu" character in Minicom.

I've implemented word-left and word-right keystrokes for compatibility
with WordStar key bindings that many CP/M application use. However, 
mathematical expressions don't really consist of "words", so the concept
might not be very useful. A "term left" and "term right" feature
would be more useful, but I haven't had time to implement it -- it would
be non-straightforward.

The whole implementation needs tidying up. I can't deny that it's a
bit of a hack.

KCalc-CPM is a big, fat lump of a thing by 8-bit standards. At least
90% of the code size is the floating-point math library and, because
this is a general-purpose utility, I have to include it all.

## Further work

There are many, many ways in which KCalc-CPM could be extended and
improved, and still fit easily in the ~60k available memory of
a typical CP/M machine (see the file TODO for examples). 
Whether I ever get around to doing any of this depends
on how much interest there is. Collaborators are always welcome.

## Legal, licence, etc.

My parts of this software are copyright (c)2021 Kevin Boone, distributed under
the terms of the GNU Public Licence (GPL) v3.0. The expression parser is based
on work by Lewis Van Winkle and distributed under the terms of a GPL-compatible
licence. The floating-point math library is, I presume, the intellectual
property of the successors in title of Manx Software Systems, who marketed the
Aztec C compiler.  I presume that permission has been given to distribute the
library in binary format as part of the general permission to distribute the
software.  


