#!/bin/sh
# This script updates syscall.h based on the contents of syscall.h.in
# essentially duplicate all the __NR_ macros with __SYS_ macros. In upstream
# this must be done by the top level Makefile.

cp syscall.h.in syscall.h
sed -n -e s/__NR_/SYS_/p < syscall.h.in >> syscall.h
