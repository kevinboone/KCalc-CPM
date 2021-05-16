# This is the Makefile for building KCalc-CPM in my emulation environment.
# To build on CP/M, use kcalc.sub (and have patience).

CPM=cpm

all: kcalc.com

kcalc.asm: kcalc.c tinyexpr.h term.h funcs.h config.h
	$(CPM) cc kcalc.c

tinyexpr.asm: tinyexpr.c tinyexpr.h config.h
	$(CPM) cc tinyexpr.c

funcs.asm: funcs.c funcs.h config.h
	$(CPM) cc funcs.c

compat.asm: compat.c compat.h config.h
	$(CPM) cc compat.c

term.asm: term.c term.h config.h
	$(CPM) cc term.c

kcalc.o: kcalc.asm
	$(CPM) as kcalc.asm

tinyexpr.o: tinyexpr.asm
	$(CPM) as tinyexpr.asm

funcs.o: funcs.asm
	$(CPM) as funcs.asm

compat.o: compat.asm
	$(CPM) as compat.asm

term.o: term.asm
	$(CPM) as term.asm

kcalc.com: kcalc.o tinyexpr.o funcs.o compat.o term.o
	$(CPM) ln kcalc.o tinyexpr.o funcs.o compat.o term.o m.lib c.lib 

clean:
	rm -f kcalc.com *.asm *.o
