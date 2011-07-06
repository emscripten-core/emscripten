# -----------------------------------------------------------------------------
# lex_object.py
# -----------------------------------------------------------------------------
import sys

if ".." not in sys.path: sys.path.insert(0,"..")
import ply.lex as lex

class CalcLexer:
    tokens = (
        'NAME','NUMBER',
        'PLUS','MINUS','TIMES','DIVIDE','EQUALS',
        'LPAREN','RPAREN',
        )

    # Tokens

    t_PLUS    = r'\+'
    t_MINUS   = r'-'
    t_TIMES   = r'\*'
    t_DIVIDE  = r'/'
    t_EQUALS  = r'='
    t_LPAREN  = r'\('
    t_RPAREN  = r'\)'
    t_NAME    = r'[a-zA-Z_][a-zA-Z0-9_]*'

    def t_NUMBER(self,t):
        r'\d+'
        try:
            t.value = int(t.value)
        except ValueError:
            print("Integer value too large %s" % t.value)
            t.value = 0
        return t

    t_ignore = " \t"

    def t_newline(self,t):
        r'\n+'
        t.lineno += t.value.count("\n")
        
    def t_error(self,t):
        print("Illegal character '%s'" % t.value[0])
        t.lexer.skip(1)
        

calc = CalcLexer()

# Build the lexer
lex.lex(object=calc)
lex.runmain(data="3+4")




