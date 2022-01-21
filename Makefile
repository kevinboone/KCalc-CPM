# This is the Makefile for building KCalc-CPM in my emulation environment.
# To build on CP/M, use kcalc.sub (and have patience).

# To use:
#   make prepare -- unpacks the necessary parts of the Aztec compiler
#   make, make clean -- work as usual
#   make unprepare -- delete the Aztec bits

# Set the path to the CPM emulator. 
# Obtain it from here: https://github.com/jhallen/cpm
CPM=cpm

# Set the path to the zipfile containing Aztec C 1.06d. Get this
# from https://www.aztecmuseum.ca/compilers.htm#cpm
AZTECZIP=~/Downloads/az80106d.zip

SOURCES := $(shell find . -type f -name "*.c")
OBJECTS := $(patsubst %,%,$(SOURCES:.c=.o))

all: kcalc.com

prepare:
	unzip -joq $(AZTECZIP) AZ80106D/BIN80/AS.COM
	unzip -joq $(AZTECZIP) AZ80106D/BIN80/CC.COM
	unzip -joq $(AZTECZIP) AZ80106D/BIN80/CC.MSG
	unzip -joq $(AZTECZIP) AZ80106D/BIN80/LN.COM
	unzip -joq $(AZTECZIP) AZ80106D/LIB/M.LIB
	unzip -joq $(AZTECZIP) AZ80106D/LIB/C.LIB
	unzip -joq $(AZTECZIP) AZ80106D/INCLUDE/*.H

%.o: %.c *.h
	$(CPM) cc -DCPM $<
	$(CPM) as $(basename $<)

kcalc.com: $(OBJECTS) 
	$(CPM) ln -o kcalc.com *.o m.lib c.lib 

clean:
	rm -f kcalc.com *.asm *.o CC.MSG *.deps

unprepare:
	rm -f AS.COM LN.COM CC.COM *.LIB *.H


