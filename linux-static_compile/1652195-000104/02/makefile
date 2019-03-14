###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test.o
TARGET	:= test

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
	$(CC) --static -o $@ ${OBJS} ${LIBS}
	
#clean
clean:
	rm -fr *o
	rm -fr $(TARGET)
