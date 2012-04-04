AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

## Define an executable target, which will be installed into the
## directory named by the predefined variable $(bindir).
bin_PROGRAMS = vodeox

## Define the list of source files for the "vodeox" target. The file extension
## .cc is recognized by Automake, and causes it to produce rules which invoke
## the C++ compiler to produce an object file (.o) from each source file. The
## header files (.h) do not result in object files by themselves, but will be
## included in distribution archives of the project.
vodeox_SOURCES = src/server/base/Logger.h src/server/base/Logger.cpp src/server/main/main.cpp

## Define an independent executable script for inclusion in the distribution
## archive. It will not be installed on an end user's system, however.
dist_noinst_SCRIPTS = autogen.sh
