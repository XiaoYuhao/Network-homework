###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= tcp_server1.o tcp_client1.o
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
	$(CC)  -o tcp_server1 tcp_server1.o ${LIBS}
	$(CC)  -o tcp_client1 tcp_client1.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr tcp_server1 tcp_client1
	

	
