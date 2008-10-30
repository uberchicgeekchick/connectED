Summary: A PHP source editor for GNOME 2.
Name: GTK-PHP-IDE
Version: 0.9.91
Release: 1
License: GPL
Group: X11/Utilities
Source: GTK-PHP-IDE-%{version}.tar.gz
URL: http://www.GTK-PHP-IDE.org/
BuildRequires: gtkhtml2
BuildRoot: /var/tmp/%{name}-root               

%description
GTK-PHP-IDE is a GNOME2 editor dedicated to editing PHP files and other 
supporting files like HTML/CSS.

%prep
rm -rf $RPM_BUILD_ROOT
%setup
./configure

%build
make

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc AUTHORS ChangeLog COPYING INSTALL NEWS README

%{_bindir}/GTK-PHP-IDE
%{_datadir}/pixmaps/GTK-PHP-IDE.png
%{_datadir}/applications/GTK-PHP-IDE.desktop
%{_datadir}/GTK-PHP-IDE/php-GTK-PHP-IDE.api

%changelog
* Wed Nov 03 2004 Andy Jeffries <andy@GTK-PHP-IDE.org>
- Removed dependancy on GtkScintilla2

* Wed Jun 04 2003 Tim Jackson <tim@timj.co.uk>
- Updated to work with new build system

* Fri Jan 17 2003 Tim Jackson <tim@timj.co.uk>
- Initial RPM packaging
