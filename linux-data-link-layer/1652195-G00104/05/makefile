###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= receiver5.o sender5.o
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
	$(CC)  -o receiver5 receiver5.o ${LIBS}
	$(CC)  -o sender5 sender5.o ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -rf rsyslog
	rm -fr receiver5 sender5