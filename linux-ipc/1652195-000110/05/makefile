###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test5-1.o test5-2.o 
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
	$(CC)  -o test5-1 test5-1.o ${LIBS}
	$(CC)  -o test5-2 test5-2.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr test5-1 test5-2  