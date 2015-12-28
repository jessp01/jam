# Available build options, you will need rpm-build >= 4.0.3 for this to work.
# Example: rpmbuild -ba --with email jam.spec
#
#  Storage Options
#  ===============
#  --with email
#  --with files
#  --with snmp
#  --with spread
#  --with stomp
#  --with tokyo
#  --with zeromq2

#
# These setup the storage backends to off by default
#
%bcond_with email
%bcond_with files
%bcond_with snmp
%bcond_with spread
%bcond_with stomp
%bcond_with tokyo
%bcond_with zeromq2


# Define version and release number
%define version @PACKAGE_VERSION@
%define release 1

Name:      php-jam
Version:   %{version}
Release:   %{release}%{?dist}
Packager:  Mikko Koppanen <mkoppanen@php.net>
Summary:   PHP jam extension
License:   PHP License
Group:     Web/Applications
URL:       http://github.com/mkoppanen/php-jam
Source:    jam-%{version}.tgz
Prefix:    %{_prefix}
Buildroot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: php-devel, make, gcc, /usr/bin/phpize

%description
Monitoring extension for PHP

%package devel
Summary: Development headers for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description devel
Development headers for %{name}

### Conditional build for email
%if %{with email}
%package email
Summary: Email storage engine for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description email
%{name} backend implementation which sends email.
%endif

### Conditional build for files
%if %{with files}
%package files
Summary: File storage engine for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description files
%{name} backend implementation which stores events in files.
%endif

### Conditional build for snmp
%if %{with snmp}
%package snmp
Summary: SNMP storage engine for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description snmp
%{name} backend implementation which sends events as SNMP traps.
%endif

### Conditional build for spread
%if %{with spread}
%package spread
Summary: Spread storage engine for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description Spread
%{name} backend implementation which sends events via spread.
%endif

### Conditional build for stomp
%if %{with stomp}
%package stomp
Summary: Stomp storage engine for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description stomp
%{name} backend implementation which sends events via stomp.
%endif

### Conditional build for zeromq2
%if %{with zeromq2}
%package zeromq2
Summary: zeromq2 storage engine for %{name}
Group:   Web/Applications
Requires: %{name} = %{version}-%{release}

%description zeromq2
%{name} backend implementation which sends events via zeromq2.
%endif

%prep
%setup -q -n jam-%{version}

%build
/usr/bin/phpize && %configure -C && %{__make} %{?_smp_mflags}

# Clean the buildroot so that it does not contain any stuff from previous builds
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}

# Install the extension
%{__make} install INSTALL_ROOT=%{buildroot}

# Create the ini location
%{__mkdir} -p %{buildroot}/etc/php.d

# Preliminary extension ini
echo "extension=jam.so" > %{buildroot}/%{_sysconfdir}/php.d/jam.ini

%if %{with email}
	pushd storage/email
	/usr/bin/phpize && cp ../../config.cache . && %configure -C && %{__make} %{?_smp_mflags}
	%{__make} install INSTALL_ROOT=%{buildroot}
	popd
%endif

%if %{with files}
	pushd storage/files
	/usr/bin/phpize && cp ../../config.cache . && %configure -C && %{__make} %{?_smp_mflags}
	%{__make} install INSTALL_ROOT=%{buildroot}
	popd
%endif

%if %{with snmp}
	pushd storage/snmp
	/usr/bin/phpize && cp ../../config.cache . && %configure -C && %{__make} %{?_smp_mflags}
	%{__make} install INSTALL_ROOT=%{buildroot}
	popd
%endif

%if %{with spread}
	pushd storage/spread
	/usr/bin/phpize && cp ../../config.cache . && %configure -C && %{__make} %{?_smp_mflags}
	%{__make} install INSTALL_ROOT=%{buildroot}
	popd
%endif

%if %{with stomp}
	pushd storage/stomp
	/usr/bin/phpize && cp ../../config.cache . && %configure -C && %{__make} %{?_smp_mflags}
	%{__make} install INSTALL_ROOT=%{buildroot}
	popd
%endif

%if %{with zeromq2}
	pushd storage/zeromq2
	/usr/bin/phpize && cp ../../config.cache . && %configure -C && %{__make} %{?_smp_mflags}
	%{__make} install INSTALL_ROOT=%{buildroot}
	popd
%endif


%clean
[ "%{buildroot}" != "/" ] && %{__rm} -rf %{buildroot}

%files
%{_libdir}/php/modules/jam.so
%{_sysconfdir}/php.d/jam.ini

%files devel
%{_includedir}/php/ext/jam/php_jam.h
%{_includedir}/php/ext/jam/php_jam_storage.h

%if %{with email}
%files email
%{_libdir}/php/modules/jam_email.so
%endif

%if %{with files}
%files files
%{_libdir}/php/modules/jam_files.so
%endif

%if %{with snmp}
%files snmp
%{_libdir}/php/modules/jam_snmp.so
%endif

%if %{with spread}
%files spread
%{_libdir}/php/modules/jam_spread.so
%endif

%if %{with stomp}
%files stomp
%{_libdir}/php/modules/jam_stomp.so
%endif

%if %{with zeromq2}
%files zeromq2
%{_libdir}/php/modules/jam_zeromq2.so
%endif



%changelog
* Sat Dec 12 2009 Mikko Koppanen <mkoppanen@php.net>
 - Initial spec file
