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
vodeox_SOURCES = base/types.h base/time.h base/scoped_lock.h base/Logger.h base/Logger.cpp main/main.cpp

