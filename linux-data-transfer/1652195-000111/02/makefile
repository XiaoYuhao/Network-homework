###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= send-level-5.o send-level-4.o send-level-3.o send-level-2.o send-level-1.o \
		   recv-level-5.o recv-level-4.o recv-level-3.o recv-level-2.o recv-level-1.o
TARGET	:= main  

#compile and lib parameter
CC		:= g++
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
	$(CC)  -o send-level-1 send-level-1.o ${LIBS}
	$(CC)  -o send-level-2 send-level-2.o ${LIBS}
	$(CC)  -o send-level-3 send-level-3.o ${LIBS}
	$(CC)  -o send-level-4 send-level-4.o ${LIBS}
	$(CC)  -o send-level-5 send-level-5.o ${LIBS}
	$(CC)  -o recv-level-5 recv-level-5.o ${LIBS}
	$(CC)  -o recv-level-4 recv-level-4.o ${LIBS}
	$(CC)  -o recv-level-3 recv-level-3.o ${LIBS}
	$(CC)  -o recv-level-2 recv-level-2.o ${LIBS}
	$(CC)  -o recv-level-1 recv-level-1.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr send-level-1 send-level-2 send-level-3 send-level-4 send-level-5
	rm -fr recv-level-5 recv-level-4 recv-level-3 recv-level-2 recv-level-1
	rm -fr sender.dat receiver.dat network.dat