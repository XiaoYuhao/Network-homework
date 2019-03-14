###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test2-1.o test2-2.o test2-3.o test2-4-1.o test2-4-2.o test2-5-1.o test2-5-2.o
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
	$(CC)  -o test2-1 test2-1.o ${LIBS}
	$(CC)  -o test2-2 test2-2.o ${LIBS}
	$(CC)  -o test2-3 test2-3.o ${LIBS}
	$(CC)  -o test2-4-1 test2-4-1.o ${LIBS}
	$(CC)  -o test2-4-2 test2-4-2.o ${LIBS}
	$(CC)  -o test2-5-1 test2-5-1.o ${LIBS}
	$(CC)  -o test2-5-2 test2-5-2.o ${LIBS}
#clean
clean:
	rm -fr *o
	rm -fr test2-1 test2-2 test2-3 test2-4-1 test2-4-2 test2-5-1 test2-5-2