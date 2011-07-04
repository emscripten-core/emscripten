import sys

if ".." not in sys.path: sys.path.insert(0,"..")

from ply import lex, yacc

t_A = 'A'
t_B = 'B'
t_C = 'C'

tokens = ('A', 'B', 'C')

the_lexer = lex.lex()

def t_error(t):
    pass

def p_error(p):
    pass

def p_start(t):
    '''start : A nest C'''
    pass

def p_nest(t):
   '''nest : B'''
   print(t[-1])

the_parser = yacc.yacc(debug = False, write_tables = False)

the_parser.parse('ABC', the_lexer)
the_parser.parse('ABC', the_lexer, tracking=True)
the_parser.parse('ABC', the_lexer, tracking=True, debug=1)
