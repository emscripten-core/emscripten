#!/usr/bin/env python

'''
see emmaken.py
'''

import os, sys

def path_from_root(*pathelems):
  rootpath = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
  return os.path.join(rootpath, *pathelems)
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

emmaken = path_from_root('tools', 'emmaken.py')
exit(os.execvp('python', ['python', emmaken] + sys.argv[1:] + ['-EMMAKEN_CXX']))

