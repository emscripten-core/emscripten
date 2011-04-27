#!/usr/bin/env python

'''
see emmaken.py
'''

import os, sys

abspath = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  return os.path.join(os.path.sep, *(abspath.split(os.sep)[:-1] + list(pathelems)))
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

emmaken = path_from_root('tools', 'emmaken.py')
exit(os.execvp('python', ['python', emmaken] + sys.argv[1:] + ['-EMMAKEN_CXX']))

