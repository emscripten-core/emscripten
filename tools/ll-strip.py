#!/usr/bin/python2

import sys, re

def print_usage():
  print >> sys.stderr, "Usage: ll-strip.py file from to"
  print >> sys.stderr, "\tStrip function bodies in all ll file in the [from, to] range"
  sys.exit(1)

try:
  range_from = int(sys.argv[2])
  range_to = int(sys.argv[3])
  if range_from >= range_to:
    raise "error"
  file = open(sys.argv[1])
except:
  print_usage()

func_start = re.compile("^define\s")
func_end = re.compile("^}$")

function_counter = 0
in_function = False
line_number = 0
skip = False
dummy_stmt = "unreachable"
for orig_line in file:
  line = orig_line.strip()
  if func_start.match(line):
    if in_function:
      print >> sys.stderr, "Discovered a function inside another function!"
      sys.exit(1)
    in_function = True
    line_number = 0
    skip = False
    function_counter = function_counter + 1
  elif func_end.match(line):
    if not in_function:
      print >> sys.stderr, "Discovered a function end without a matching beginning!"
      sys.exit(1)
    in_function = False
    line_number = 0
    skip = False
  elif in_function:
    line_number = line_number + 1
    if not skip and line_number == 1:
      if line == dummy_stmt:
        function_counter = function_counter - 1
      if range_from <= function_counter <= range_to and line_number == 1:
        #import pdb;pdb.set_trace()
        if line != dummy_stmt:
          skip = True
          print dummy_stmt
  if not skip:
    print orig_line.rstrip("\n")

