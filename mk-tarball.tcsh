#!/bin/tcsh -f
set version = "0.1.0.1"
cd ..
if ( ! -d "connectED" && ! -l "connectED" ) ln -s src connectED

tar --dereference -cjf "connectED-${version}.tar.bz2" connectED
