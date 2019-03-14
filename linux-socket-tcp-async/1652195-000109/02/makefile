###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= tcp_server2-1.o tcp_client2-1.o tcp_server2-2.o tcp_client2-2.o 
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
	$(CC)  -o tcp_server2-1 tcp_server2-1.o ${LIBS}
	$(CC)  -o tcp_client2-1 tcp_client2-1.o ${LIBS}
	$(CC)  -o tcp_server2-2 tcp_server2-2.o ${LIBS}
	$(CC)  -o tcp_client2-2 tcp_client2-2.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr tcp_server2-1 tcp_client2-1 tcp_server2-2 tcp_client2-2 