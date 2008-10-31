SUBDIRS = \
	data	\
	doc	\
	pixmaps	\
	src \
	po

applicationsdir = $(datadir)/applications
applications_in_files = GTK-PHP-IDE.desktop.in
applications_DATA = $(applications_in_files:.desktop.in=.desktop)

spec_in_files = GTK-PHP-IDE.spec.in

EXTRA_DIST = \
	GTK-PHP-IDE.spec			\
	GTK-PHP-IDE.applications		\
	$(applications_in_files)	\
	$(spec_in_files)		\
	src/gtkscintilla2
