# lex_rule3.py
#
# Rule function with incorrect number of arguments

import sys
if ".." not in sys.path: sys.path.insert(0,"..")

import ply.lex as lex

tokens = [
    "PLUS",
    "MINUS",
    "NUMBER",
    ]

t_PLUS = r'\+'
t_MINUS = r'-'
def t_NUMBER(t,s):
    r'\d+'
    return t

def t_error(t):
    pass

lex.lex()


