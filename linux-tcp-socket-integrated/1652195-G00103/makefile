###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= server.o client.o
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
	$(CC)  -o server server.o ${LIBS}
	$(CC)  -o client client.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr txt
	rm -fr server client