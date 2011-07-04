# lex_re2.py
#
# Regular expression rule matches empty string

import sys
if ".." not in sys.path: sys.path.insert(0,"..")

import ply.lex as lex

tokens = [
    "PLUS",
    "MINUS",
    "NUMBER",
    ]

t_PLUS = r'\+?'
t_MINUS = r'-'
t_NUMBER = r'(\d+)'

def t_error(t):
    pass



lex.lex()


