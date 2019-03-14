###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= receiver4.o sender4.o
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
	$(CC)  -o receiver4 receiver4.o ${LIBS}
	$(CC)  -o sender4 sender4.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -rf rsyslog
	rm -fr receiver4 sender4