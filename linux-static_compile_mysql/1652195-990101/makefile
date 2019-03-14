###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:=
TARGET	:= mysql_demo

#compile and lib parameter
CC		:= g++
LIBS	:= -L/usr/lib64/mysql -lmysqlclient -lpthread -lz -lm -ldl -lssl -lcrypto
LDFLAGS	:= -L.
DEFINES	:=
INCLUDE := -I/usr/include/mysql
CFLAGS	:=
CXXFLAGS:=

.PHONY: clean

#link parameter
LIB := 

#link
$(TARGET):mysql_demo.cpp
	$(CC) --static -o $@  $(INCLUDE) $^ $(LIBS)
	
#clean
clean:
	rm -fr *o
	rm -fr $(TARGET)