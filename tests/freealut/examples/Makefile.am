# Build, but do not install the following test programs:
noinst_PROGRAMS = hello_world playfile

# We need to link against our *own* libalut.
LDADD = ../src/libalut.la

# Specifying the following path is needed to find <AL/alut.h>.
AM_CPPFLAGS = -I$(top_srcdir)/include
