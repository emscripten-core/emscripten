# lex_module.py
#

import sys
if ".." not in sys.path: sys.path.insert(0,"..")

import ply.lex as lex
import lex_module_import
lex.lex(module=lex_module_import)
lex.runmain(data="3+4")
