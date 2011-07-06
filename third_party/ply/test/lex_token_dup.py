# lex_token_dup.py
#
# Duplicate token name in tokens

import sys
if ".." not in sys.path: sys.path.insert(0,"..")

import ply.lex as lex

tokens = [
    "PLUS",
    "MINUS",
    "NUMBER",
    "MINUS"
    ]

t_PLUS = r'\+'
t_MINUS = r'-'

def t_NUMBER(t):
    r'\d+'
    return t

def t_error(t):
    pass

lex.lex()


