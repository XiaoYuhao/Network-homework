###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= receiver1.o sender1.o
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
	$(CC)  -o receiver1 receiver1.o ${LIBS}
	$(CC)  -o sender1 sender1.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -rf rsyslog
	rm -fr receiver1 sender1