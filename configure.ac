## Helloworld C++: an example project using Automake

AC_INIT([vodeox], [0.1], [igmorv@gmail.com],
        [vodeox], [http://www.w3.org])
AC_PREREQ([2.59])

AC_CONFIG_HEADERS([config.h])
# check for C++ preprocessor and compiler
AC_PROG_CXXCPP
AC_PROG_CXX

# automake initialization (mandatory) including a check for automake API version >= 1.10
AM_INIT_AUTOMAKE([1.10 -Wall no-define])

# use the C++ compiler for the following checks
AC_LANG([C++])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([iostream])

# LibEvent
LIBEVENT_MINIMUM=2.0.10
AC_SUBST(LIBEVENT_MINIUM)

PKG_CHECK_MODULES(LIBEVENT, [libevent >= $LIBEVENT_MINIMUM])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/server/Makefile])

AC_OUTPUT
