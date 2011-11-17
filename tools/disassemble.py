#!/usr/bin/python

import sys, shared, io, subprocess

cmd = [shared.LLVM_DIS, '-o=-'] + shared.LLVM_DIS_OPTS

reader = shared.get_std_reader()
writer = shared.get_std_writer()

# Check for BC
if shared.is_bitcode(reader):
	process = subprocess.Popen(cmd, stdin=reader, stdout=writer)
	process.communicate()
	if process.returncode != 0:
		raise RuntimeError('Could not disassemble stream.')
else:
	shared.delegate(reader, writer)

