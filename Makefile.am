SUBDIRS = \
	data	\
	doc	\
	pixmaps	\
	src \
	po

applicationsdir = $(datadir)/applications
applications_in_files = GTK_PHP_IDE.desktop.in
applications_DATA = $(applications_in_files:.desktop.in=.desktop)

spec_in_files = GTK_PHP_IDE.spec.in

EXTRA_DIST = \
	GTK_PHP_IDE.spec			\
	GTK_PHP_IDE.applications		\
	$(applications_in_files)	\
	$(spec_in_files)		\
	src/gtkscintilla2
