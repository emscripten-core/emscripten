# -----------------------------------------------------------------------------
# yacc_rr_unused.py
#
# A grammar with reduce/reduce conflicts and a rule that never
# gets reduced.
# -----------------------------------------------------------------------------
import sys

if ".." not in sys.path: sys.path.insert(0,"..")
import ply.yacc as yacc

tokens = ('A', 'B', 'C')

def p_grammar(p):
   '''
   rule1 : rule2 B
         | rule2 C

   rule2 : rule3 B
         | rule4
         | rule5

   rule3 : A

   rule4 : A

   rule5 : A
   '''

yacc.yacc()
