BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
Summary:  test-1652195 rpm package
Name:	  test-1652195
Version:  0.1
Release:  1
Source:   test-1652195-0.1.tar.gz
License:  GPL
Packager: amoblin
Group:    Application


%description
This is a software for test-1652195 rmp package

%prep
%autosetup -n test-1652195-0.1

%build
make %{?_smp_mflags}

%install
RPM_INSTALL_ROOT=$RPM_BUILD_ROOT make install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr (754,root,root,754) 
/usr/sbin/test-1652195
/usr/sbin/test-service.sh
/usr/lib/systemd/system/test-1652195.service
/usr/1652195/
/usr/1652195/1652195.dat
/etc/1652195.conf
/usr/lib64/lib1652195.so


%pre
echo "准备安装test-1652195"

%post
echo "成功安装test-1652195"

%preun
echo "准备卸载test-1652195"

%postun
echo "成功卸载test-1652195"


