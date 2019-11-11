Name:       libfyaml
Version:    0.3
Release:    1%{?dist}
Summary:    Fully feature complete YAML 1.2 parser and emitter, supporting the latest YAML spec and passing the full YAML testsuite

License:    MIT
URL:        https://github.com/pantoniou/libfyaml
Source0:    https://github.com/pantoniou/libfyaml/archive/v%{version}.tar.gz

BuildRequires:  gcc

%description
A fancy 1.2 YAML and JSON parser/writer.
Fully feature complete YAML parser and emitter, supporting the latest YAML
spec and passing the full YAML testsuite. It is designed to be very efficient,
avoiding copies of data, and has no artificial limits like the 1024 character
limit for implicit keys.
The libfyaml is using https://github.com/yaml/yaml-test-suite as a core part
of it's testsuite.


%package devel
Summary:   Development files for libfyaml applications
Requires:  libfyaml = %{version}-%{release}, pkgconfig, libtool


%package tool
Summary:   A YAML manipulation tool is included in libfyaml, aptly name fy-tool
Requires:  libfyaml = %{version}-%{release}, pkgconfig


%description devel
The %{name}-devel package contains libraries and header files for
developing applications that use libfyaml.


%description tool
A YAML manipulation tool is included in libfyaml, aptly name fy-tool. It's a multi tool application,
acting differently according to the name it has when it's invoked. There are four tool modes, namely:
fy-testsuite: Used for outputing a test-suite specific event stream which is used for comparison with
              the expected output of the suite.
fy-dump:      General purpose YAML parser and dumper, with syntax coloring support, visible whitespace
              options, and a number of output modes.
fy-filter:    YAML filtering tool allows to extract information out of a YAML document.
fy-join:      YAML flexible join tool.


%prep
%setup -q -n %{name}-%{version}


%build
echo "%{version}" > .tarball-version
./bootstrap.sh
%configure
make %{?_smp_mflags}


%install
rm -rf %{buildroot}
make DESTDIR=%{buildroot} INSTALL="install -p" install
rm -f %{buildroot}%{_libdir}/*.{la,a}

#soname=$(readelf -d %{buildroot}%{_libdir}/libyaml.so | awk '$2 == "(SONAME)" {print $NF}' | tr -d '[]')
#rm -f %{buildroot}%{_libdir}/libyaml.so
#echo "INPUT($soname)" > %{buildroot}%{_libdir}/libyaml.so


%check
make check


%ldconfig_scriptlets


%files
%license LICENSE
%doc README.md
%{_libdir}/%{name}*.so.*


%files devel
#%doc doc/html
%{_libdir}/%{name}*.so
%{_libdir}/pkgconfig/%{name}.pc
%{_includedir}/%{name}.h


%files tool
%{_bindir}/fy-tool
%{_bindir}/fy-dump
%{_bindir}/fy-filter
%{_bindir}/fy-join
%{_bindir}/fy-testsuite


%changelog
* Sun Nov 10 2019 Evgenii Kolesnikov <ekolesni@redhat.com> - 0.3-1
- Initial packaging
