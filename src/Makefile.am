bin_PROGRAMS = @PROGRAM_NAME@
static_CFLAGS=-g3 -ggdb

INCLUDES =					\
	$(connectED_CFLAGS)			\
	$(static_CFLAGS)			\
	-I$(top_srcdir)/src/gtkscintilla2	\
	-DGTK -DSCI_LEXER			\
	-DCONNECTED_DATA_DIR=\""$(datadir)"\"	\
	-DCONNECTED_PREFIX=\""$(prefix)"\"	\
	-DPIXMAP_DIR=\""$(datadir)/pixmaps"\"	\


connectED_SOURCES =						\
	calltip.c			calltip.h		\
	classbrowser.c			classbrowser.h		\
	classbrowser_parse.c		classbrowser_parse.h	\
	edit_template.c			edit_template.h		\
	find_replace.c			find_replace.h		\
	connectED_ipc.c			connectED_ipc.h		\
	grel2abs.c			grel2abs.h		\
	plugin.c			plugin.h		\
	main.c				main.h			\
	main_window.c			main_window.h		\
	main_window_callbacks.c		main_window_callbacks.h	\
	main_window_menu.h		main_window.typedef.h	\
	preferences.c			preferences.h		\
	preferences_dialog.c		preferences_dialog.h	\
	syntax_check.c			syntax_check.h		\
	tab.c				tab.h			\
	tab_cxx.c			tab_cxx.h		\
	tab_css.c			tab_css.h		\
	tab_perl.c			tab_perl.h		\
	tab_php.c			tab_php.h		\
	tab_python.c			tab_python.h		\
	tab_sql.c			tab_sql.h		\
	tab_util.c			tab_util.h		\
	templates.c			templates.h		\



gtkscintilla2/libgtkscintilla2.a:
	cd gtkscintilla2/scintilla/gtk && $(MAKE) && cd ../../..
	cd gtkscintilla2 && $(MAKE) libgtkscintilla2.a && cd ..

connectED_DEPENDENCIES =			\
	gtkscintilla2/libgtkscintilla2.a	\
	gtkscintilla2/scintilla/bin/scintilla.a

connectED_LDADD =			\
	$(connectED_LIBS)		\
	$(connectED_DEPENDENCIES)

gtkscintilla2/scintilla/bin/scintilla.a: connectED_DEPENDENCIES

gtkscintilla2/libgtkscintilla2.a:
	cd gtkscintilla2/scintilla/gtk && $(MAKE) && cd ../../..
	cd gtkscintilla2 && $(MAKE) libgtkscintilla2.a && cd ..

