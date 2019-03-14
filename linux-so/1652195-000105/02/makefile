###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:=
TARGET	:=test2

#compile and lib parameter
CC		:=g++
LIBS	:=-ltest1
LDFLAGS	:=-L.
DEFINES	:=
INCLUDE :=-I.
CFLAGS	:=
CXXFLAGS:=

.PHONY: clean

#link parameter
LIB := libtest1.so

#link
$(TARGET):test2.o $(LIB)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)
	
$(LIB):test1.o
	$(CC) -shared -o -fPIC -o $@ $^
	

#complie
test1.o:test1.cpp
	$(CC) -c -fPIC $^ -o $@

#clean
clean:
	rm -fr *o
	rm -fr $(TARGET)
	