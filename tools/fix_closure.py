#!/usr/bin/env python

'''
With very very large projects, closure compiler can translate FUNCTION_TABLE into something like

  J = [0, 0, func, 0, f(), 0, function() { ... }, 0, ..]

where f() returns a new empty function, and after it is an inlined function. This inlining can be of very very large functions, in which case it can make the source unparsable by any JS engine due to "too much recursion" or "Maximum call stack size exceeded".

This script uninlines those functions. Note that we can't do this using Uglify, since these scripts can't be parsed by it either!
'''

import os, sys, re

infile = open(sys.argv[1], 'r')
outfile = open(sys.argv[2], 'w')

class ObjectParser:
  def read(self, s, line):
    '''
      Read an element of the FUNCTION_TABLE until the end (a comma or the end of FUNCTION_TABLE), returning that location
    '''
    assert line[s-1] == ',' # we are a new element, after a comma
    curly = 0
    paren = 0
    string = 0
    is_func = 0
    i = s
    while True:
      c = line[i]
      if not string:
        if c == '"' or c == "'":
          string = 1
        elif c == '{':
          is_func = 1
          curly += 1
        elif c == '}':
          curly -= 1
        elif c == '(':
          paren += 1
        elif c == ')':
          paren -= 1
        elif not curly and not paren:
          if c in [',', ']']:
            return (i, is_func and line[s:i].startswith('function('))
      else:
        if c == '"' or c == "'":
          string = 0
      i += 1

lines = infile.readlines()
i = 0
while i < len(lines):
  line = lines[i]
  curr = line.find('=[0,0,')
  if curr > 0:
    # a suspect
    target = line[curr-1]
    curr += 5
    parser = ObjectParser()
    add = []
    while line[curr] != ']':
      assert line[curr] == ','
      curr += 1
      next, is_func = parser.read(curr, line)
      if is_func:
        text = line[curr:next]
        assert text.startswith('function(')
        ident = 'uninline_' + target + '_' + str(curr) # these idents should be unique, but might in theory collide with the rest of the JS code! XXX
        line = line[:curr] + ident + line[next:]
        add += 'function ' + ident + '(' + text[len('function('):]
      while line[curr] != ',' and line[curr] != ']':
        curr += 1
    lines[i] = line
    lines = lines[:i] + add + lines[i:]
    i += len(add)
  i += 1

for line in lines:
  outfile.write(line)
outfile.close()

