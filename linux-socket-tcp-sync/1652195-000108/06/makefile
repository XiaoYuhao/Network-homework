###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= tcp_server6-1.o tcp_client6-1.o tcp_server6-2.o tcp_client6-2.o tcp_server6-3.o tcp_client6-3.o tcp_server6-4.o tcp_client6-4.o
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
	$(CC)  -o tcp_server6-1 tcp_server6-1.o ${LIBS}
	$(CC)  -o tcp_client6-1 tcp_client6-1.o ${LIBS}
	$(CC)  -o tcp_server6-2 tcp_server6-2.o ${LIBS}
	$(CC)  -o tcp_client6-2 tcp_client6-2.o ${LIBS}
	$(CC)  -o tcp_server6-3 tcp_server6-3.o ${LIBS}
	$(CC)  -o tcp_client6-3 tcp_client6-3.o ${LIBS}	
	$(CC)  -o tcp_server6-4 tcp_server6-4.o ${LIBS}
	$(CC)  -o tcp_client6-4 tcp_client6-4.o ${LIBS}	
	
#clean
clean:
	rm -fr *o 
	rm -fr tcp_server6-1 tcp_client6-1 tcp_server6-2 tcp_client6-2 tcp_server6-3 tcp_client6-3 tcp_server6-4 tcp_client6-4
	

	
