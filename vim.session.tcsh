#!/bin/tcsh -f
set my_editor = "`printf "${0}" | sed 's/.*\/\([^\.]\+\).*/\1/g'`"
switch ( "${my_editor}" )
case "gedit":
	breaksw
case "vim":
default:
	set my_editor = `printf "%s -p '%s %s'" "vim" '+tabdo' '$-2'`
	breaksw
endsw

${my_editor} "./src/main_window.c" "./src/prefix.h" "./src/main_window.h" "./src/Makefile"
