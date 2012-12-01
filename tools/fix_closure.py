#!/usr/bin/env python2

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
    #print 'zz start parsing!', line[s-1:s+100]
    curly = 0
    paren = 0
    string = 0
    is_func = 0
    i = s
    while True:
      c = line[i]
      #print 'parsing! CPSF', c, curly, paren, string, is_func
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
            #print 'zz done,', line[s:i], line[s:i].startswith('function(')
            return (i, is_func and line[s:i].startswith('function('))
      else:
        if c == '"' or c == "'":
          string = 0
      i += 1

line = infile.read()
curr = 0
while True:
  curr = line.find('=[0,0,', curr)
  if curr < 0: break
  # a suspect
  #print 'zz suspect!', curr, line[curr-10:curr+10]
  target = line[curr-1]
  curr += 5
  parser = ObjectParser()
  add = []
  while line[curr] != ']':
    assert line[curr] == ','
    curr += 1
    while line[curr] in ['\n', ' ']:
      curr += 1
    next, is_func = parser.read(curr, line)
    if is_func:
      text = line[curr:next]
      #print 'zz func!', text
      assert text.startswith('function(')
      ident = 'zzz_' + target + '_' + hex(curr)[2:] # these idents should be unique, but might in theory collide with the rest of the JS code! XXX
      line = line[:curr] + ident + line[next:]
      add += 'function ' + ident + '(' + text[len('function('):] + '\n'
      #print 'zz after func fix:', line[curr:curr+100]
    while line[curr] != ',' and line[curr] != ']':
      curr += 1
  #print 'zz exited:', line[curr:curr+100]
  curr += 1
  assert line[curr] == ';', 'odd char: ' + str([line[curr], line[curr-10:curr+10]])
  curr += 1
  line = line[:curr] + '\n' + ''.join(add) + line[curr:]

outfile.write(line)
outfile.close()

