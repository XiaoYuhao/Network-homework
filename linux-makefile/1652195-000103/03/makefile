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

main: test1.c test2.c test3.c
	gcc test1.c -o test1
	gcc test2.c -o test2
	gcc test3.c -o test3
	
test1:test1.c 
	gcc test1.c -o test1
test2:test2.c 
	gcc test2.c -o test2
test3:test3.c
	gcc test3.c -o test3
	
#clean
clean:
	rm -f test1 test2 test3 ${OBJS}