#!/usr/bin/python

import sys, shared, io, subprocess

cmd = [shared.LLVM_AS, '-o=-']

reader = shared.get_std_reader()
writer = shared.get_std_writer()

# Check for BC
if shared.is_bitcode(reader):
	shared.delegate(reader, writer)
else:
	process = subprocess.Popen(cmd, stdin=reader, stdout=writer)
	process.communicate()
	if process.returncode != 0:
		raise RuntimeError('Could not assemble stream')


