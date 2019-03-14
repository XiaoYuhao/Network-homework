###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= receiver2.o sender2.o
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
	$(CC)  -o receiver2 receiver2.o ${LIBS}
	$(CC)  -o sender2 sender2.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -rf rsyslog
	rm -fr receiver2 sender2