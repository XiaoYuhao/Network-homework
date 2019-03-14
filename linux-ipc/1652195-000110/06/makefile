###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test6-1-1.o test6-1-2.o 
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
	$(CC)  -o test6-1-1 test6-1-1.o ${LIBS}
	$(CC)  -o test6-1-2 test6-1-2.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr test6-1-1 test6-1-2 