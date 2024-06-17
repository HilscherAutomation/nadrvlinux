#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.65)
AC_INIT([libnetana],[1.0.6.0])
AM_INIT_AUTOMAKE(@PACKAGE_NAME@, @PACKAGE_VERSION@)
AC_CONFIG_HEADER([config.h])


AC_ARG_ENABLE(debug, 
            AS_HELP_STRING([--enable-debug], [Compile application in debug mode]),
            [
                CFLAGS="$CFLAGS -O0 -g3 -DDEBUG"
                AC_MSG_RESULT([******************])
                AC_MSG_RESULT([*** Debug Mode ***])
                AC_MSG_RESULT([******************])
            ],
            [
                CFLAGS="$CFLAGS -O2"
            ])

AC_ARG_ENABLE(verbose, 
            AS_HELP_STRING([--enable-verbose], [Enable console debugging outputs of library]),
            [
                CFLAGS="$CFLAGS -DVERBOSE"
            ])

# Checks for programs.
dnl Check for install
AC_PROG_INSTALL
if test -z "${INSTALL}"; then
  AC_MSG_ERROR([install could not be found, please install e.g.: GNU coreutils])
fi

dnl Check for gcc
AC_PROG_CC
if test -z "$CC"; then
  AC_MSG_ERROR([gcc could not be found, please install])
fi

dnl Check for libtool
LT_INIT
if test -z "$LIBTOOL"; then
  AC_MSG_ERROR([libtool could not be found, please install])
fi

AC_PROG_LN_S

dnl Check for make
AC_PATH_PROGS(MAKE, gmake gnumake make,, $PATH)
if test -z "$MAKE"; then
  AC_MSG_ERROR([make could not be found, please install])
fi

dnl Check if gcc supports -fno-stack-protector
if test "x$ac_cv_c_compiler_gnu" = xyes; then
  AC_CACHE_CHECK([whether gcc has -fno-stack-protector],
                 no_stack_protector_flag_cv_, 
                 [
                  saved_CFLAGS=$CFLAGS
                  CFLAGS=" $CFLAGS -fno-stack-protector"
                  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[no_stack_protector_flag_cv_=yes],[no_stack_protector_flag_cv_=no])
                  CFLAGS=$saved_CFLAGS
                  ])
  if test "x$no_stack_protector_flag_cv_" = xyes; then
    CFLAGS="$CFLAGS -fno-stack-protector"
  fi
fi

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_C_VOLATILE

#AC_C_BIGENDIAN(
#  [
#    CFLAGS="$CFLAGS -DCIFX_TOOLKIT_BIGENDIAN"
#  ])


# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([clock_gettime getpagesize gettimeofday memmove memset munmap strdup strncasecmp strstr])






dnl
dnl versions
dnl
VERSION_MAJOR=` echo "$PACKAGE_VERSION" | \
  sed -e "s/-/./g" | \
  (IFS=. read VERSION_MAJOR VERSION_MINOR VERSION_MICRO VERSION_EXTRA; echo $VERSION_MAJOR)`
VERSION_MINOR=` echo "$PACKAGE_VERSION" | \
  sed -e "s/-/./g" | \
  (IFS=. read VERSION_MAJOR VERSION_MINOR VERSION_MICRO VERSION_EXTRA; echo $VERSION_MINOR)`
VERSION_MICRO=` echo "$PACKAGE_VERSION" | \
  sed -e "s/-/./g" | \
  (IFS=. read VERSION_MAJOR VERSION_MINOR VERSION_MICRO VERSION_EXTRA; echo $VERSION_MICRO)`
VERSION_EXTRA=` echo "$PACKAGE_VERSION" | \
  sed -e "s/-/./g" | \
  (IFS=. read VERSION_MAJOR VERSION_MINOR VERSION_MICRO VERSION_EXTRA; echo $VERSION_EXTRA)`

if test -n "$VERSION_EXTRA"; then
  VERSION_EXTRA_SEP="."
else
  VERSION_EXTRA_SEP=""
fi

AC_SUBST(PACKAGE_VERSION)
AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_MINOR)
AC_SUBST(VERSION_MICRO)
AC_SUBST(VERSION_EXTRA_SEP)
AC_SUBST(VERSION_EXTRA)

AC_CONFIG_FILES([Makefile netana.pc])
AC_OUTPUT
