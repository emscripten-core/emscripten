# lex_token5.py
#
# Return a bad token name

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

def t_NUMBER(t):
    r'\d+'
    t.type = "NUM"
    return t

def t_error(t):
    pass

lex.lex()
lex.input("1234")
t = lex.token()


