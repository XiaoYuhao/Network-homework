###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= tcp_server5.o tcp_client5-1.o tcp_client5-2.o 
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
	$(CC)  -o tcp_server5 tcp_server5.o ${LIBS}
	$(CC)  -o tcp_client5-1 tcp_client5-1.o ${LIBS}
	$(CC)  -o tcp_client5-2 tcp_client5-2.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr tcp_server5 tcp_client5-1 tcp_client5-2