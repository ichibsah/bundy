SUBDIRS = . tests

AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib
AM_CPPFLAGS += $(BOOST_INCLUDES) $(MULTITHREADING_FLAG)
AM_CPPFLAGS += -I$(top_srcdir)/src/lib/dns -I$(top_builddir)/src/lib/dns
AM_CPPFLAGS += -I$(top_srcdir)/src/lib/nsas -I$(top_builddir)/src/lib/nsas
AM_CPPFLAGS += -I$(top_srcdir)/src/lib/cache -I$(top_builddir)/src/lib/cache
AM_CPPFLAGS += $(SQLITE_CFLAGS)
AM_CXXFLAGS = $(B10_CXXFLAGS)

# Some versions of GCC warn about some versions of Boost regarding
# missing initializer for members in its posix_time.
# https://svn.boost.org/trac/boost/ticket/3477
# But older GCC compilers don't have the flag.
AM_CXXFLAGS += $(WARNING_NO_MISSING_FIELD_INITIALIZERS_CFLAG)

if USE_CLANGPP
# clang++ complains about unused function parameters in some boost header
# files.
AM_CXXFLAGS += -Wno-unused-parameter
endif

lib_LTLIBRARIES = libcache.la
libcache_la_SOURCES  = resolver_cache.h resolver_cache.cc
libcache_la_SOURCES  += message_cache.h message_cache.cc
libcache_la_SOURCES  += message_entry.h message_entry.cc
libcache_la_SOURCES  += rrset_cache.h rrset_cache.cc
libcache_la_SOURCES  += rrset_entry.h rrset_entry.cc
libcache_la_SOURCES  += cache_entry_key.h cache_entry_key.cc
libcache_la_SOURCES  += rrset_copy.h rrset_copy.cc
libcache_la_SOURCES  += local_zone_data.h local_zone_data.cc

CLEANFILES = *.gcno *.gcda