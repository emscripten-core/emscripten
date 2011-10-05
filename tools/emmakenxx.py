#!/usr/bin/env python

'''
see emmaken.py
'''

import os, sys

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)
exec(open(path_from_root('tools', 'shared.py'), 'r').read())

emmaken = path_from_root('tools', 'emmaken.py')
os.environ['EMMAKEN_CXX'] = '1'
exit(os.execvp('python', ['python', emmaken] + sys.argv[1:]))

