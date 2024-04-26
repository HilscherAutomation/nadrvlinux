NETANA_LIBRARY_VERSION = 1:5:0

pkgconfigdir = $(libdir)/pkgconfig

INCLUDES = -I$(srcdir) -I$(srcdir)/sys -I$(srcdir)/../netanalyzer_kernel_mod/toolkit/Include/

lib_LTLIBRARIES = libnetana.la
libnetana_la_SOURCES = netana_functions.c netana_capture.c netana_helper.c

libnetana_la_CFLAGS  = -fPIC -Wall -DUSER_SPACE @CFLAGS@ 
libnetana_la_LDFLAGS = -version-info $(NETANA_LIBRARY_VERSION) -lpthread @LDFLAGS@

library_include_HEADERS = $(srcdir)/../netanalyzer_kernel_mod/toolkit/Include/netana_errors.h $(srcdir)/../netanalyzer_kernel_mod/toolkit/Include/netana_user.h $(srcdir)/libnetana.h
library_includedir =$(includedir)/netana
