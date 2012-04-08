AUTOMAKE_OPTIONS = subdir-objects

ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

## Define an executable target, which will be installed into the
## directory named by the predefined variable $(bindir).
bin_PROGRAMS = vodeox client_test

AM_CFLAGS = \
    @LIBEVENT_CFLAGS@

apps_ldadd = \
    @LIBEVENT_LIBS@

## Define the list of source files for the "vodeox" target. The file extension
## .cc is recognized by Automake, and causes it to produce rules which invoke
## the C++ compiler to produce an object file (.o) from each source file. The
## header files (.h) do not result in object files by themselves, but will be
## included in distribution archives of the project.
vodeox_SOURCES = base/types.h base/time.h base/time.cpp base/scoped_lock.h base/comcurrent_queue.h base/Logger.h base/Logger.cpp base/threadpool.h base/threadpool.cpp main/main.cpp
vodeox_LDADD = ${apps_ldadd}


client_test_SOURCES = tests/client_test.cpp
client_test_LDADD = ${apps_ldadd}
