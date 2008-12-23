#!/bin/tcsh -f
set version = "0.1.0.1"
cd ..
if ( ! -d "GTK-PHP-IDE" && ! -l "GTK-PHP-IDE" ) ln -s src GTK-PHP-IDE

tar --dereference -cjf "GTK-PHP-IDE-${version}.tar.bz2" GTK-PHP-IDE
