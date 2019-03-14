###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= tcp_server4-1.o tcp_client4-1-1.o tcp_client4-1-2.o tcp_server4-2.o tcp_client4-2-1.o tcp_client4-2-2.o
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
	$(CC)  -o tcp_server4-1 tcp_server4-1.o ${LIBS}
	$(CC)  -o tcp_client4-1-1 tcp_client4-1-1.o ${LIBS}
	$(CC)  -o tcp_client4-1-2 tcp_client4-1-2.o ${LIBS}
	$(CC)  -o tcp_server4-2 tcp_server4-2.o ${LIBS}
	$(CC)  -o tcp_client4-2-1 tcp_client4-2-1.o ${LIBS}
	$(CC)  -o tcp_client4-2-2 tcp_client4-2-2.o ${LIBS}
	
#clean
clean:
	rm -fr *o 
	rm -fr tcp_server4-1 tcp_client4-1-1 tcp_client4-1-2 tcp_server4-2 tcp_client4-2-1 tcp_client4-2-2
	

	
