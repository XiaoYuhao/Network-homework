###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= receiver3.o sender3.o
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
	$(CC)  -o receiver3 receiver3.o ${LIBS}
	$(CC)  -o sender3 sender3.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -rf rsyslog
	rm -fr receiver3 sender3