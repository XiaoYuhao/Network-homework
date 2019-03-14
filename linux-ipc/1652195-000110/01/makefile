###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test1-1.o test1-2.o test1-3.o
TARGET	:= main

#compile and lib parameter
CC		:= gcc
LIBS	:= -lm
LDFLAGS	:= -L.
DEFINES	:=
INCLUDE := 
CFLAGS	:=
CXXFLAGS:=

.PHONY: clean

#link parameter
LIB := 

#link
$(TARGET):${OBJS}
	$(CC)  -o test1-1 test1-1.o ${LIBS}
	$(CC)  -o test1-2 test1-2.o ${LIBS}	
	$(CC)  -o test1-3 test1-3.o ${LIBS}	
#clean
clean:
	rm -fr *o
	rm -fr test1-1 test1-2 test1-3