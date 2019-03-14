###########################
#	Makefile
###########################

#source object target
SOURCE 	:=
OBJS 	:= test.o
TARGET	:= test

#compile and lib parameter
CC		:= gcc
LIBS	:= -l1652195
LDFLAGS	:= -L.
DEFINES	:=
INCLUDE := 
CFLAGS	:=
CXXFLAGS:=


.PHONY: clean
.PHONY: install
.PHONY: uninstall
.PHONY: rpm

#link parameter
LIB := lib1652195.so

#link
$(TARGET):${OBJS} $(LIB)
	$(CC)  -o $@ ${OBJS} ${LIB}
	
$(LIB):lib1652195.o
	$(CC) -shared -o -fPIC -o $@ $^
	
#complie
lib1652195.o:readconfig.c
	$(CC) -c -fPIC $^ -o $@
	
#clean
clean:
	rm -fr *o
	rm -fr ${TARGET}
	rm -fr $(LIB)
	rm -fr *rpm
	
#install
install:
	mkdir -p $(RPM_INSTALL_ROOT)/usr/sbin/
	mkdir -p $(RPM_INSTALL_ROOT)/usr/lib64/
	mkdir -p $(RPM_INSTALL_ROOT)/etc/
	mkdir -p $(RPM_INSTALL_ROOT)/usr/lib/systemd/system/
	cp -f test $(RPM_INSTALL_ROOT)/usr/sbin/test-1652195
	cp -f lib1652195.so $(RPM_INSTALL_ROOT)/usr/lib64/lib1652195.so
	mkdir -p $(RPM_INSTALL_ROOT)/usr/1652195
	cp -f 1652195.dat $(RPM_INSTALL_ROOT)/usr/1652195/1652195.dat
	cp -f 1652195.conf $(RPM_INSTALL_ROOT)/etc/1652195.conf
	cp -f test-1652195.service $(RPM_INSTALL_ROOT)/usr/lib/systemd/system/test-1652195.service
	cp -f test-service.sh $(RPM_INSTALL_ROOT)/usr/sbin/test-service.sh
	
#rpm:
rpm:
	cp -f test-1652195.spec /root/rpmbuild/SPECS/test-1652195.spec
	cd .. && cp -r 1652195-000107 test-1652195-0.1
	cd .. && tar -cz test-1652195-0.1  > /root/rpmbuild/SOURCES/test-1652195-0.1.tar.gz
	cd .. && rm -rf test-1652195-0.1
	cd /root/rpmbuild/SPECS && rpmbuild -ba test-1652195.spec
	cp -f /root/rpmbuild/RPMS/x86_64/test-1652195-0.1-1.x86_64.rpm test-1652195.rpm

#uninstall
uninstall:
	rm -fr /usr/sbin/test-1652195
	rm -fr /usr/lib64/lib1652195.so
	rm -fr /etc/1652195.conf
	rm -fr /usr/1652195/1652195.dat
	rm -fr /usr/lib/systemd/system/test-1652195.service
	rm -fr /usr/sbin/test-service.sh
	rm -fr /root/rpmbuild/SPECS/test-1652195.spec
	rm -fr /root/rpmbuild/SOURCES/test-1652195-0.1.tar.gz
	rm -fr /root/rpmbuild/BUILD/test-1652195-0.1
	rm -fr /root/rpmbuild/RPMS/x86_64/test-1652195-0.1-1.x86_64.rpm
	rmdir /usr/1652195
	
	
