###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= receiver6.o sender6.o
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
	$(CC)  -o receiver6 receiver6.o ${LIBS}
	$(CC)  -o sender6 sender6.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -rf rsyslog
	rm -fr receiver6 sender6