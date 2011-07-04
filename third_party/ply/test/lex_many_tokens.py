# lex_many_tokens.py
#
# Test lex's ability to handle a large number of tokens (beyond the
# 100-group limit of the re module)

import sys
if ".." not in sys.path: sys.path.insert(0,"..")

import ply.lex as lex

tokens = ["TOK%d" % i for i in range(1000)]

for tok in tokens:
    if sys.version_info[0] < 3:
        exec("t_%s = '%s:'" % (tok,tok))
    else:
        exec("t_%s = '%s:'" % (tok,tok), globals())

t_ignore = " \t"

def t_error(t):
    pass

lex.lex(optimize=1,lextab="manytab")
lex.runmain(data="TOK34: TOK143: TOK269: TOK372: TOK452: TOK561: TOK999:")


