#!/bin/tcsh -f
set version = "0.1.1"
cd "`dirname '${0}'`"/..
if ( ! -d "connectED" && ! -l "connectED" ) ln -s src connectED
tar --dereference -cjf "connectED-${version}.tar.bz2" connectED
