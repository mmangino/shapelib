Name: ruby-shapelib
Version: 0.6
Release: 1p
Summary: ruby binding for ShapeLib
License: LGPL or MIT-like
Group: Library/File_Format
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildPrereq: ruby, shapelib
Prereq: ruby, shapelib

%description
ruby-shapelib is a Ruby binding of Shapefile C library.
Users can read/write shapefile from Ruby script.

%prep
%setup
./extconf.rb

%build
make

%install
make install DESTDIR=${RPM_BUILD_ROOT}

%post

%clean
rm -rf ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
/usr/lib/site_ruby/1.8/i386-linux-gnu/shapelib.so

%changelog
* Tue Mar 07 2006 prasinos <prasinos@users.sourceforge.net> (0.6)
- code style improvement:
  - avoid prototype for static function
  - avoid int/unsigned mixture

* Fri Feb 17 2006 prasinos <prasinos@users.sourceforge.net> (0.4)
- bugfix: name compare error
- bugfix: nil was erroneously converted to NODATA for x/y/z
- documentation refleshed

* Thu Feb 16 2006 prasinos <prasinos@users.sourceforge.net> (0.3)
- most of features implemented

* Tue Feb 14 2006 prasinos <prasinos@users.sourceforge.net> (0.2)
- initial RPM package

