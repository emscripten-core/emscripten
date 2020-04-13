#!/bin/sh
# This scripe updates syscall.h based on the contents of syscall.h.in
# essentially duplicate all the __NR_ marcros with __SYS_ macros.  In upstream
# must this is don by the top level Makefile.

cp syscall.h.in syscall.h
sed -n -e s/__NR_/SYS_/p < syscall.h.in >> syscall.h
