###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test3-1-1.o test3-1-2.o test3-2-1.o test3-2-2.o
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
	$(CC)  -o test3-1-1 test3-1-1.o ${LIBS}
	$(CC)  -o test3-1-2 test3-1-2.o ${LIBS}
	$(CC)  -o test3-2-1 test3-2-1.o ${LIBS}
	$(CC)  -o test3-2-2 test3-2-2.o ${LIBS}
#clean
clean:
	rm -fr *o
	rm -fr test3-1-1 test3-1-2 test3-2-1 test3-2-2