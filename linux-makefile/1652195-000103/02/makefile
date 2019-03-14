###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test1.o test2.o test3.o
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
	$(CC) -o $@ ${OBJS} ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr $(TARGET)