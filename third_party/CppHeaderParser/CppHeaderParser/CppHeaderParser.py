#!/usr/bin/python2
#
# Author: Jashua R. Cloutier (contact via sourceforge username:senexcanis)
#
# Copyright (C) 2010, Jashua R. Cloutier
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
#
# * Neither the name of Jashua R. Cloutier nor the names of its
#   contributors may be used to endorse or promote products derived from
#   this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#
# The CppHeaderParser.py script is written in Python 2.4 and released to
# the open source community for continuous improvements under the BSD
# 2.0 new license, which can be found at:
#
#   http://www.opensource.org/licenses/bsd-license.php
#
"""
CppHeaderParser2.0: April 2011 - August 2011
    by HartsAntler
    http://pyppet.blogspot.com

    Quick Start - User API:
        h = CppHeaderParser.CppHeader("someheader.h")
        for name in h.classes:
            c = h.classes[name]
            for method in c['methods']['public']:
                print method['name']
                print dir(method)        # view the rest of the API here.

        ... TODO document more ...



    New Features by Hart:
        should be able to parse all c++ files, not just headers
        parsing global typedefs with resolution
        parsing global structs
        fixes nested struct in class changes accessor type
        parsing if class is abstract
        parsing more info about variables
        save ordering of classes, structs, and typedefs
        handle forward decl of class in a class
        handle mutable, static, and other variable types
        handle 1D arrays
        handle throw keyword and function prefix __attribute__((__const__))
        handle nameless parameters "void method(void);"
        handle simple templates, and global functions.

    Internal Developer Notes:

        1. double name stacks:
            . the main stack is self.nameStack, this stack is simpler and easy to get hints from
            . the secondary stack is self.stack is the full name stack, required for parsing somethings
            . each stack maybe cleared at different points, since they are used to detect different things
            . it looks ugly but it works :)

        2. Had to make the __repr__ methods simple because some of these dicts are interlinked.
            For nice printing, call something.show()

"""

import ply.lex as lex
import os
import sys
import re

import inspect

def lineno():
    """Returns the current line number in our program."""
    return inspect.currentframe().f_back.f_lineno

version = __version__ = "1.9.9o"

tokens = [
    'NUMBER',
    'NAME',
    'OPEN_PAREN',
    'CLOSE_PAREN',
    'OPEN_BRACE',
    'CLOSE_BRACE',
    'COLON',
    'SEMI_COLON',
    'COMMA',
    'COMMENT_SINGLELINE',
    'COMMENT_MULTILINE',
    'PRECOMP_MACRO',
    'PRECOMP_MACRO_CONT', 
    'ASTERISK',
    'AMPERSTAND',
    'EQUALS',
    'MINUS',
    'PLUS',  
    'DIVIDE', 
    'CHAR_LITERAL', 
    'STRING_LITERAL',
    'OPERATOR_DIVIDE_OVERLOAD', 
    'NEW_LINE',

    'OPEN_BRACKET',
    'CLOSE_BRACKET',

]

t_OPEN_BRACKET = r'\['
t_CLOSE_BRACKET = r'\]'


#t_ignore = " \t\r[].|!?%@"			# (cppheaderparser 1.9x)
#t_ignore = " \t\r[].|!?%@'^\\"
t_ignore = " \t\r.|!?%@'^\\"
t_NUMBER = r'[0-9][0-9XxA-Fa-f]*'
t_NAME = r'[<>A-Za-z_~][A-Za-z0-9_]*'
t_OPERATOR_DIVIDE_OVERLOAD = r'/='
t_OPEN_PAREN = r'\('
t_CLOSE_PAREN = r'\)'
t_OPEN_BRACE = r'{'
t_CLOSE_BRACE = r'}'
t_SEMI_COLON = r';'
t_COLON = r':'
t_COMMA = r','
t_PRECOMP_MACRO = r'\#.*'
t_PRECOMP_MACRO_CONT = r'.*\\\n'
def t_COMMENT_SINGLELINE(t):
    r'\/\/.*\n'
    global doxygenCommentCache
    if t.value.startswith("///") or t.value.startswith("//!"):
        if doxygenCommentCache:
            doxygenCommentCache += "\n"
        if t.value.endswith("\n"):
            doxygenCommentCache += t.value[:-1]
        else:
            doxygenCommentCache += t.value
t_ASTERISK = r'\*'
t_MINUS = r'\-'
t_PLUS = r'\+'
t_DIVIDE = r'/[^/]'         # fails to catch "/("   -  method operator that overloads divide
t_AMPERSTAND = r'&'
t_EQUALS = r'='
t_CHAR_LITERAL = "'.'"
#found at http://wordaligned.org/articles/string-literals-and-regular-expressions
#TODO: This does not work with the string "bla \" bla"
t_STRING_LITERAL = r'"([^"\\]|\\.)*"'
#Found at http://ostermiller.org/findcomment.html
def t_COMMENT_MULTILINE(t):
    r'/\*([^*]|[\r\n]|(\*+([^*/]|[\r\n])))*\*+/'
    global doxygenCommentCache
    if t.value.startswith("/**") or t.value.startswith("/*!"):
        #not sure why, but get double new lines
        v = t.value.replace("\n\n", "\n")
        #strip prefixing whitespace
        v = re.sub("\n[\s]+\*", "\n*", v)
        doxygenCommentCache += v
def t_NEWLINE(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

def t_error(v):
    print( "Lex error: ", v )

lex.lex()
debug = 0
debug_trace = 0
def trace_print(*arg):
    global debug_trace
    if debug_trace: print(arg)

supportedAccessSpecifier = [
    'public',
    'protected', 
    'private'
]

enumMaintianValueFormat = False
doxygenCommentCache = ""

def is_namespace(nameStack):
    """Determines if a namespace is being specified"""
    if len(nameStack) == 0:
        return False
    if nameStack[0] == "namespace":
        return True
    return False

def is_enum_namestack(nameStack):
    """Determines if a namestack is an enum namestack"""
    if len(nameStack) == 0:
        return False
    if nameStack[0] == "enum":
        return True
    if len(nameStack) > 1 and nameStack[0] == "typedef" and nameStack[1] == "enum":
        return True
    return False

class CppParseError(Exception): pass
    

class _CppClass(dict):
	def _parser_helper( self, stack ):
		prev = None
		prev2 = None
		print('stack IN', ' '.join(stack))
		for i, tok in enumerate(stack):	# can not trust the first single ":" or last
			if prev and prev2 and tok == ':' and prev != ':' and prev2 != ':':
				break
			prev = tok
			prev2 = prev


		a = stack[ : i+1 ]
		b = stack[ i+1 : ]
		while a[-1] == ':': a.pop()

		print( 'HEAD', a )
		print('______________')
		print( 'TAIL', b )

		if ''.join(stack).replace('::','_').count(':') >= 2:
			if stack.count('class') == 1:
				self['name'] = stack[ stack.index('class') + 1 ]
			elif stack.count('struct') == 1:
				self['name'] = stack[ stack.index('struct') + 1 ]
			else:
				self['unsafe_template'] = True
			b = []

		elif a[0] == 'template' and ('class' in a or 'struct' in a):
			if '>' not in a:
				self['name'] = a[ a.index('class') + 1 ]
				self['unsafe_template'] = True

			else:
				copy = list( a )
				last = len(a) - 1 - a[::-1].index('>')
				self['template_typename'] = a[ a.index('>')-1 ]
				a = a[ last+1 : ]
				if not a:
					a = copy[ copy.index('class')+1 : ]
					x = ''.join( a )
					assert '<' in x and '>' in x
					self['name'] = x
					self['special_template'] = True

				elif 'class' in a:
					self['name'] = ''.join( a[1:] )

				elif 'struct' in a:
					self['name'] = ''.join( a[1:] )
					self['struct'] = True
					self['struct_template'] = self['template_typename']
				elif 'class' in b:
					self['name'] = b[ b.index('class') + 1 ]
					b = []
				elif 'struct' in b:
					self['name'] = b[ b.index('struct') + 1 ]
					b = []
				else:
					self['unsafe_template'] = True
					assert 0

		elif a[0] == 'template' and b[-2] in ('class','struct'):
			self['name'] = b[-1]
			b = []	# b is invalid
		elif a[0] == 'class':
			self['name'] = ''.join( a[1:] )
		elif 'class' in b:
			self['name'] = b[ b.index('class') + 1 ]
			b = []
		elif 'struct' in b:
			self['name'] = b[ b.index('struct') + 1 ]
			self['struct'] = True
			b = []
		else:
			assert 0


		if b:
			p = [ {'access':'public', 'class':''} ]
			for x in b:
				if x in 'public protected private'.split():
					p[-1]['access'] = x
				elif x == 'virtual':
					p[-1]['virtual'] = True
				elif x == ',':
					p.append( {'access':'public', 'class':''} )
				else:
					p[-1]['class'] += x
			self['inherits'] = p
		else:
			self['inherits'] = []

		return True


class CppClass( _CppClass ):
    """Takes a name stack and turns it into a class
    
    Contains the following Keys:
    self['name'] - Name of the class
    self['doxygen'] - Doxygen comments associated with the class if they exist
    self['inherits'] - List of Classes that this one inherits where the values
        are of the form {"access": Anything in supportedAccessSpecifier
                                  "class": Name of the class
    self['methods'] - Dictionary where keys are from supportedAccessSpecifier
        and values are a lists of CppMethod's
    self['properties'] - Dictionary where keys are from supportedAccessSpecifier
        and values are lists of CppVariable's 
    self['enums'] - Dictionary where keys are from supportedAccessSpecifier and
        values are lists of CppEnum's
    self['structs'] - Dictionary where keys are from supportedAccessSpecifier and
        values are lists of nested Struct's
    
    An example of how this could look is as follows:
    #self =
    {
        'name': ""
        'inherits':[]
        'methods':
        {
            'public':[],
            'protected':[], 
            'private':[]
        }, 
        'properties':
        {
            'public':[],
            'protected':[], 
            'private':[]
        },
        'enums':
        {
            'public':[],
            'protected':[], 
            'private':[]
        }
    }
    """
    def __repr__( self ): return self['name']

    def get_all_methods(self):
        r = []
        for typ in 'public protected private'.split(): r += self['methods'][typ]
        return r

    def get_all_method_names( self ):
        r = []
        for typ in 'public protected private'.split(): r += self.get_method_names(typ)        # returns list
        return r

    def get_all_pure_virtual_methods( self ):
        r = {}
        for typ in 'public protected private'.split(): r.update(self.get_pure_virtual_methods(typ))        # returns dict
        return r


    def get_method_names( self, type='public' ): return [ meth['name'] for meth in self['methods'][ type ] ]

    def get_pure_virtual_methods( self, type='public' ):
        r = {}
        for meth in self['methods'][ type ]:
            if meth['pure_virtual']: r[ meth['name'] ] = meth
        return r

    def __init__(self, nameStack):
        self['nested_classes'] = []
        self['parent'] = None
        self['abstract'] = False
        self['namespace'] = ""
        self._public_enums = {}
        self._public_structs = {}
        self._public_typedefs = {}
        self._public_forward_declares = []

        if (debug): print( "Class:   ",  nameStack )
        if (len(nameStack) < 2):
            print( "Error detecting class" )
            return
        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""

        methodAccessSpecificList = {}
        propertyAccessSpecificList = {}
        enumAccessSpecificList = {}
        structAccessSpecificList = {}
        typedefAccessSpecificList = {}
        forwardAccessSpecificList = {}
        
        for accessSpecifier in supportedAccessSpecifier:
            methodAccessSpecificList[accessSpecifier] = []
            propertyAccessSpecificList[accessSpecifier] = []
            enumAccessSpecificList[accessSpecifier] = []
            structAccessSpecificList[accessSpecifier] = []
            typedefAccessSpecificList[accessSpecifier] = []
            forwardAccessSpecificList[accessSpecifier] = []

        self['methods'] = methodAccessSpecificList
        self['properties'] = propertyAccessSpecificList
        self['enums'] = enumAccessSpecificList
        self['structs'] = structAccessSpecificList
        self['typedefs'] = typedefAccessSpecificList
        self['forward_declares'] = forwardAccessSpecificList

        ok = self._parser_helper( nameStack )
        if not ok: self['invalid'] = True

    def show_all(self):
        self.show()
        for key in self.keys(): print( '	%s : %s' %(key,self[key]) )
 
    def show(self):
        """Convert class to a string"""
        namespace_prefix = ""
        if self["namespace"]: namespace_prefix = self["namespace"] + "::"
        rtn = "class %s"%(namespace_prefix + self["name"])
        if self['abstract']: rtn += '    (abstract)\n'
        else: rtn += '\n'

        if 'doxygen' in self.keys(): rtn += self["doxygen"] + '\n'
        if 'parent' in self.keys() and self['parent']: rtn += 'parent class:' + self['parent'] + '\n'

        if "inherits" in self.keys():
            rtn += "  Inherits: "
            for inheritClass in self["inherits"]:
                rtn += "%s %s, "%(inheritClass["access"], inheritClass["class"])
            rtn += "\n"
        rtn += "  {\n"
        for accessSpecifier in supportedAccessSpecifier:
            rtn += "    %s\n"%(accessSpecifier)
            #Enums
            if (len(self["enums"][accessSpecifier])):
                rtn += "        <Enums>\n"
            for enum in self["enums"][accessSpecifier]:
                rtn += "            %s\n"%(repr(enum))
            #Properties
            if (len(self["properties"][accessSpecifier])):
                rtn += "        <Properties>\n"
            for property in self["properties"][accessSpecifier]:
                rtn += "            %s\n"%(repr(property))
            #Methods
            if (len(self["methods"][accessSpecifier])):
                rtn += "        <Methods>\n"
            for method in self["methods"][accessSpecifier]:
                rtn += "\t\t" + method.show() + '\n'
        rtn += "  }\n"
        print( rtn )

class _CppMethod( dict ):
    def _params_helper1( self, stack ):    
        # new July 7th, deal with defaults that init: vec3(0,0,0)
        # so that comma split still works later on parsing the parameters.

        # also deal with "throw" keyword
        if 'throw' in stack: stack = stack[ : stack.index('throw') ]

        ## remove GCC keyword __attribute__(...) and preserve returns ##
        cleaned = []
        hit = False; hitOpen = 0; hitClose = 0
        for a in stack:
            if a == '__attribute__': hit = True
            if hit:
                if a == '(': hitOpen += 1
                elif a == ')': hitClose += 1
                if a==')' and hitOpen == hitClose:
                    hit = False
            else:
                cleaned.append( a )
        stack = cleaned

        # also deal with attribute((const)) function prefix #
        # TODO this needs to be better #
        if len(stack) > 5:
            a = ''.join(stack)
            if a.startswith('((__const__))'): stack = stack[ 5 : ]
            elif a.startswith('__attribute__((__const__))'): stack = stack[ 6 : ]

        stack = stack[stack.index('(') + 1: ]
        if not stack: return []
        if len(stack)>=3 and stack[0]==')' and stack[1]==':':    # is this always a constructor?
            self['constructor'] = True
            return []

        stack.reverse(); _end_ = stack.index(')'); stack.reverse()
        stack = stack[ : len(stack)-(_end_+1) ]

        if '(' not in stack: return stack    # safe to return, no defaults that init a class
        elif stack.index('(') > stack.index(')'):  # deals with: "constructor(int x) : func(x) {}"
            return stack[ : stack.index(')') ]       # fixed july20

        # transforms ['someclass', '(', '0', '0', '0', ')'] into "someclass(0,0,0)'"
        r = []; hit=False
        for a in stack:
            if a == '(': hit=True
            elif a == ')': hit=False
            if hit or a == ')': r[-1] = r[-1] + a
            else: r.append( a )
        return r

    def _params_helper2( self, params ):
        for p in params:
            # if param becomes unresolved - function/parent is marked with 'unresolved_parameters'
            if 'function' in self: p['function'] = self
            else: p['method'] = self
            # force full namespace for nested items, or take method name space as our own (bad idea?)
            if '::' in p['type']:
                ns = p['type'].split('::')[0]
                if ns not in Resolver.NAMESPACES and ns in Resolver.CLASSES:
                    p['type'] = self['namespace'] + p['type']
            else: p['namespace'] = self[ 'namespace' ]

class CppMethod( _CppMethod ):
    """Takes a name stack and turns it into a method
    
    Contains the following Keys:
    self['returns'] - Return type of the method (ex. "int")
    self['name'] - Name of the method (ex. "getSize")
    self['doxygen'] - Doxygen comments associated with the method if they exist
    self['parameters'] - List of CppVariables
    """
    def show(self):
        r = ['method name: %s (%s)' %(self['name'],self['debug']) ]
        if self['returns']: r.append( 'returns: %s'%self['returns'] )
        if self['parameters']: r.append( 'number arguments: %s' %len(self['parameters']))
        if self['pure_virtual']: r.append( 'pure virtual: %s'%self['pure_virtual'] )
        if self['constructor']: r.append( 'constructor' )
        if self['destructor']: r.append( 'destructor' )
        return '\n\t\t  '.join( r )

    def __init__(self, nameStack, curClass=None, methinfo={} ):
        if (debug): print( "Method:   ",  nameStack )
        global doxygenCommentCache

        if not curClass: self['function'] = True

        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""
        if "operator" in nameStack:
            self["name"] = "".join(nameStack[nameStack.index('operator'):nameStack.index('(')])
        else:
            self["name"] = " ".join(nameStack[nameStack.index('(') - 1:nameStack.index('(')])

        self.update( methinfo )    # harts hack
        paramsStack = self._params_helper1( nameStack )
        params = []
        #See if there is a doxygen comment for the variable
        doxyVarDesc = {}
        #TODO: Put this into a class
        if self.has_key("doxygen"):
            doxyLines = self["doxygen"].split("\n")
            lastParamDesc = ""
            for doxyLine in doxyLines:
                if " @param " in doxyLine or " \param " in doxyLine:
                    try:
                        #Strip out the param
                        doxyLine = doxyLine[doxyLine.find("param ") + 6:]
                        (var, desc) = doxyLine.split(" ", 1)
                        doxyVarDesc[var] = desc.strip()
                        lastParamDesc = var
                    except: pass
                elif " @return " in doxyLine or " \return " in doxyLine:
                    lastParamDesc = ""
                    # not handled for now
                elif lastParamDesc:
                    try:
                        doxyLine = doxyLine.strip()
                        if " " not in doxyLine:
                            lastParamDesc = ""
                            continue
                        doxyLine = doxyLine[doxyLine.find(" ") + 1:]
                        doxyVarDesc[lastParamDesc] += " " + doxyLine
                    except: pass
        
        #Create the variable now
        while (len(paramsStack)):
            if (',' in paramsStack):
                param = CppVariable(paramsStack[0:paramsStack.index(',')],  doxyVarDesc=doxyVarDesc)
                if len(param.keys()): params.append(param)
                paramsStack = paramsStack[paramsStack.index(',') + 1:]
            else:
                param = CppVariable(paramsStack,  doxyVarDesc=doxyVarDesc)
                if len(param.keys()): params.append(param)
                break

        self["parameters"] = params
        self._params_helper2( params )    # mods params inplace


class _CppVariable(dict):
    def _name_stack_helper( self, stack, fullStack ):
        print('V'*80); print( stack ); print(fullStack); print('_'*80)
        stack = list(stack)
        if stack[-1].isdigit() and '=' not in stack:        # TODO refactor me - was: '=' not in stack or 
            # check for array[n] and deal with funny array syntax: "int myvar:99"
            bits = []
            while stack and stack[-1].isdigit(): bits.append( stack.pop() )
            if bits:
                bits.reverse()
                self['bitfield'] = int(''.join(bits))
            assert stack[-1] == ':'
            stack.pop()

        ## find and strip array def ##
        if '[' in stack:
            assert stack.count('[') == stack.count(']')
            a = ['']; hit = 0; _stack = []
            for s in stack:
                if s == '[': hit += 1
                elif s == ']': hit -= 1; a.append( '' )
                elif hit: a[-1] += s
                elif not hit: _stack.append( s )
            stack = _stack

            b = []
            for s in a:
                if s.isdigit(): b.append( int( s ) )
                elif s != '': self['invalid'] = True
            if not b: self['pointer'] += 1
            else:
                self['array'] = b[0]
                self['array_dimensions'] = b
                if len(b)>1: self['multidimensional'] = True

        while stack and not stack[-1]: stack.pop()            # can be empty?
        return stack



class CppVariable( _CppVariable ):
    """Takes a name stack and turns it into a method
    
    Contains the following Keys:
    self['type'] - Type for the variable (ex. "const string &")
    self['raw_type'] - Type of variable without pointers or other markup (ex. "string")
    self['name'] - Name of the variable (ex. "numItems")
    self['namespace'] - Namespace containing the enum
    self['desc'] - Description of the variable if part of a method (optional)
    self['doxygen'] - Doxygen comments associated with the method if they exist
    self['defalt'] - Default value of the variable, this key will only exist if there is a default value
    """
    Vars = []

    def __init__(self, nameStack,  fullStack=None, doxyVarDesc=None):	# CppMethod will not pass fullStack for params 
        self['aliases'] = []; self['parent'] = None; self['typedef'] = None
        for key in 'constant reference pointer static typedefs class fundamental unresolved mutable'.split():
            self[ key ] = 0


        _stack_ = nameStack
        nameStack = self._name_stack_helper( nameStack, fullStack )
        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""

        if (debug): print( "Variable: ",  nameStack )

        if (len(nameStack) < 2):
            if len(nameStack) == 1: self['type'] = nameStack[0]; self['name'] = ''
            else: print(_stack_); assert 0

        elif ("=" in nameStack):
            self["type"] = " ".join(nameStack[:nameStack.index("=") - 1])
            self["name"] = nameStack[nameStack.index("=") - 1]
            self['default'] = " ".join(nameStack[nameStack.index("=") + 1:])
            self['default'] = self['default'].replace(' <', '<' )
            self['default'] = self['default'].replace(' >', '>' )

        elif nameStack[-1] in '*&':        # rare cases - function param is an unnamed pointer: "void somemethod( SomeObject* )"
            self['type'] = ' '.join(nameStack)
            self['name'] = ''

        else:    # common case
            self["type"] = " ".join(nameStack[:-1])
            self["name"] = nameStack[-1]

        self["type"] = self["type"].replace(" :",":")
        self["type"] = self["type"].replace(": ",":")
        self["type"] = self["type"].replace(" <","<")
        self["type"] = self["type"].replace(" >",">")
        #Optional doxygen description
        if doxyVarDesc and self['name'] in doxyVarDesc:
            self['description'] = doxyVarDesc[ self['name'] ]

        self['type'] = self['type'].strip()
        a = []
        for b in self['type'].split():
            if b == '__const__': b = 'const'
            a.append( b )

        if not a:
            self['invalid'] = True		# void someinvalidfunction( int x, y=INVALID );
            print('WARN - bad variable', self )

        else:
            if a[0] == 'class':
                self['explicit_class'] = a[1]
                a = a[1:]
            elif a[0] == 'struct':
                self['explicit_struct'] = a[1]
                a = a[1:]
        self['type'] = ' '.join( a )

        if self['name'].count('<') != self['name'].count('>'): self['invalid'] = True

        CppVariable.Vars.append( self )        # save and resolve later

class _CppEnum(dict):
    def resolve_enum_values( self, values ):
        """Evaluates the values list of dictionaries passed in and figures out what the enum value
        for each enum is editing in place:
        
        Example:
        From: [{'name': 'ORANGE'},
               {'name': 'RED'},
               {'name': 'GREEN', 'value': '8'}]
        To:   [{'name': 'ORANGE', 'value': 0},
               {'name': 'RED', 'value': 1},
               {'name': 'GREEN', 'value': 8}]
        """
        t = 'int'; i = 0
        names = [ v['name'] for v in values ]
        for v in values:
            if 'value' in v:
                a = v['value'].strip()
                if a.lower().startswith("0x"):
                    try:
                        i = a = int(a , 16)
                    except:pass
                elif a.isdigit():
                    i = a = int( a )
                elif a in names:
                    for other in values:
                        if other['name'] == a:
                            v['value'] = other['value']
                            break

                elif '"' in a or "'" in a: t = 'char*'          # only if there are quotes it this a string enum
                else:
                    try:
                        a = i = ord(a)
                    except: pass
                if not enumMaintianValueFormat: v['value'] = a
            else: v['value'] = i
            i += 1
        return t

class CppEnum(_CppEnum):
    """Takes a name stack and turns it into an Enum
    
    Contains the following Keys:
    self['name'] - Name of the enum (ex. "ItemState")
    self['namespace'] - Namespace containing the enum
    self['values'] - List of values where the values are a dictionary of the
        form {"name": name of the key (ex. "PARSING_HEADER"),
                  "value": Specified value of the enum, this key will only exist
                    if a value for a given enum value was defined
                }
    """
    def __init__(self, nameStack):
        if len(nameStack) < 4 or "{" not in nameStack or "}" not in nameStack:
            #Not enough stuff for an enum
            return
        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""
        valueList = []
        #Figure out what values it has
        valueStack = nameStack[nameStack.index('{') + 1: nameStack.index('}')]
        while len(valueStack):
            tmpStack = []
            if "," in valueStack:
                tmpStack = valueStack[:valueStack.index(",")]
                valueStack = valueStack[valueStack.index(",") + 1:]
            else:
                tmpStack = valueStack
                valueStack = []
            d = {}
            if len(tmpStack) == 1: d["name"] = tmpStack[0]
            elif len(tmpStack) >= 3 and tmpStack[1] == "=":
                d["name"] = tmpStack[0]; d["value"] = " ".join(tmpStack[2:])
            elif len(tmpStack) == 2 and tmpStack[1] == "=":
                if (debug): print( "WARN-enum: parser missed value for %s"%tmpStack[0] )
                d["name"] = tmpStack[0]

            if d: valueList.append( d )

        if len(valueList):
            self['type'] = self.resolve_enum_values( valueList )    # returns int for standard enum
            self["values"] = valueList
        else:
            print( 'WARN-enum: empty enum', nameStack )
            return
        #Figure out if it has a name
        preBraceStack = nameStack[:nameStack.index("{")]
        postBraceStack = nameStack[nameStack.index("}") + 1:]
        if (len(preBraceStack) == 2 and "typedef" not in nameStack):
            self["name"] = preBraceStack[1]           
        elif len(postBraceStack) and "typedef" in nameStack:
                self["name"] = " ".join(postBraceStack)
        else: print( 'WARN-enum: nameless enum', nameStack )
        #See if there are instances of this
        if "typedef" not in nameStack and len(postBraceStack):
            self["instances"] = []
            for var in postBraceStack:
                if "," in var:
                    continue
                self["instances"].append(var)
        self["namespace"] = ""

def is_fundamental(s):
    for a in s.split():
        if a not in 'size_t wchar_t struct union unsigned signed bool char short int float double long void *': return False
    return True

def prune_templates( stack ):
    x = []; hit = 0
    for a in stack:
        if a == '<' or a.startswith('<'): hit += 1
        elif a == '>' or a.endswith('>'): hit -= 1
        elif not hit and a != 'template': x.append( a )
    return x

def prune_arrays( stack ):
    x = []; hit = 0
    for a in stack:
        if a == '[' or a.startswith('['): hit += 1
        elif a == ']' or a.endswith(']'): hit -= 1
        elif not hit: x.append( a )
    return x


def is_method_namestack(stack):
    clean = prune_templates( stack ); print('CLEAN TEMPLATES',clean)
    clean = prune_arrays( clean ); print('CLEAN ARRAYS',clean)

    r = False
    if 'operator' in stack: r = True    # allow all operators
    elif not ('(' in stack or '/(' in stack): r = False
    elif stack[0]=='mutable': r = False
    elif clean and clean[0] in ('class', 'struct'): r = False
    elif not ('(' in clean or '/(' in clean): r = False
    #elif '__attribute__' in stack: r = False
    #elif stack[0] == '__attribute': r = False
    elif stack[0] == 'typedef': r = False    # TODO deal with typedef function prototypes
    elif stack[0] in 'return if else case switch throw +'.split(): print( stack ); assert 0; r = False
    elif stack[0] == '}' and stack[1] in 'return if else case switch'.split(): print( stack ); assert 0; r = False
    elif '=' in stack:# and stack.index('=') < stack.index('('):
        #if 'template' not in stack: r = False
        if '=' in clean and clean.index('=') < clean.index('('): r = False
        else: r = True

    elif '{' in stack and stack.index('{') < stack.index('('): r = False    # struct that looks like a method/class
    elif '(' in stack and ')' in stack:
        if '{' in stack and '}' in stack: r = True
        #elif '/' in stack: r = False
        #elif '/ ' in stack: r = False
        elif stack[-1] == ';': r = True
        elif '{' in stack: r = True
        x = ''.join(stack)
        if x.endswith('(0,0,0);'): r = False
        elif x.endswith('(0);'): r = False
        elif x.endswith(',0);'): r = False
        elif x.endswith(',1);'): r = False
        elif x.endswith(',true);'): r = False
        elif x.endswith(',false);'): r = False
        elif x.endswith(',0xFF);'): r = False
    else: r = False
    print( 'is method namestack', r, stack ); print('_'*80)
    return r


class CppStruct(dict):
    Structs = []
    def __init__(self, nameStack):
        if nameStack[0] == 'template': self['template'] = True
        if nameStack.index('struct')+1 < len(nameStack):
            self['type'] = nameStack[ nameStack.index('struct') + 1 ]
        else: self['type'] = None
        self['fields'] = []
        self['methods'] = []
        self['parent'] = None
        self.Structs.append( self )

C99_NONSTANDARD = {
    'int8' : 'signed char',
    'int16' : 'short int',
    'int32' : 'int',
    'int64' : 'int64_t',        # this can be: long int (64bit), or long long int (32bit)
    'uint' : 'unsigned int',
    'uint8' : 'unsigned char',
    'uint16' : 'unsigned short int',
    'uint32' : 'unsigned int',
    'uint64' : 'uint64_t',    # depends on host bits
}


def standardize_fundamental( s ):
    if s in C99_NONSTANDARD: return C99_NONSTANDARD[ s ]
    else: return s


class Resolver(object):
    C_FUNDAMENTAL = 'size_t unsigned signed bool char wchar short int float double long void'.split()
    C_FUNDAMENTAL += 'struct union enum'.split()


    SubTypedefs = {}        # TODO deprecate?
    NAMESPACES = []
    CLASSES = {}
    STRUCTS = {}

    def initextra(self):
        self.typedefs = {}
        self.typedefs_info = {}
        self.typedefs_order = []
        self.classes_order = []
        self.template_classes = {}
        self.template_typedefs = {}
        self.structs = Resolver.STRUCTS
        self.structs_order = []
        self.namespaces = Resolver.NAMESPACES        # save all namespaces
        self.curStruct = None
        self.stack = []    # full name stack, good idea to keep both stacks? (simple stack and full stack)
        self._classes_brace_level = {}    # class name : level
        self._structs_brace_level = {}        # struct type : level
        self._method_body = None
        self._forward_decls = []
        self._template_typenames = []    # template<typename XXX>
        self.functions = []     # free functions

    def current_namespace(self): return self.cur_namespace(True)

    def cur_namespace(self, add_double_colon=False):
        rtn = ""
        i = 0
        while i < len(self.nameSpaces):
            rtn += self.nameSpaces[i]
            if add_double_colon or i < len(self.nameSpaces) - 1: rtn += "::"
            i+=1
        return rtn


    def guess_ctypes_type( self, string ):
        pointers = string.count('*')
        string = string.replace('*','')

        a = string.split()
        if 'unsigned' in a: u = 'u'
        else: u = ''
        if 'long' in a and 'double' in a: b = 'longdouble'    # there is no ctypes.c_ulongdouble (this is a 64bit float?)
        elif a.count('long') == 2 and 'int' in a: b = '%sint64' %u
        elif a.count('long') == 2: b = '%slonglong' %u
        elif 'long' in a: b = '%slong' %u
        elif 'double' in a: b = 'double'    # no udouble in ctypes
        elif 'short' in a: b = '%sshort' %u
        elif 'char' in a:
            if u: b = 'ubyte'	# no ctypes.c_uchar
            else: b = 'char'
        elif 'wchar' in a: b = 'wchar'
        elif 'bool' in a: b = 'bool'
        elif 'float' in a: b = 'float'

        elif 'int' in a: b = '%sint' %u
        elif 'int8' in a: b = 'int8'
        elif 'int16' in a: b = 'int16'
        elif 'int32' in a: b = 'int32'
        elif 'int64' in a: b = 'int64'

        elif 'uint' in a: b = 'uint'
        elif 'uint8' in a: b = 'uint8'
        elif 'uint16' in a: b = 'uint16'
        elif 'uint32' in a: b = 'uint32'
        elif 'uint64' in a: b = 'uint64'

        elif 'size_t' in a: b = 'size_t'
        elif 'void' in a: b = 'void_p'

        elif string in 'struct union'.split(): b = 'void_p'    # what should be done here? don't trust struct, it could be a class, no need to expose via ctypes
        else: b = 'void_p'

        if not pointers: return 'ctypes.c_%s' %b
        else:
            x = ''
            for i in range(pointers): x += 'ctypes.POINTER('
            x += 'ctypes.c_%s' %b
            x += ')' * pointers
            return x

    def resolve_type( self, string, result ):    # recursive
        '''
        keeps track of useful things like: how many pointers, number of typedefs, is fundamental or a class, etc...
        '''
        ## be careful with templates, what is inside <something*> can be a pointer but the overall type is not a pointer
        ## these come before a template
        s = string.split('<')[0]
        if not result['constant']: result[ 'constant' ] = 'const' in s.split()
        if not result['static']: result[ 'static' ] = 'static' in s.split()
        if not result['mutable']: result[ 'mutable' ] = 'mutable' in s.split()

        ## these come after a template
        s = string.split('>')[-1]
        result[ 'pointer' ] += s.count('*')
        result[ 'reference' ] += s.count('&')


        x = string; alias = False
        for a in '* & const static mutable'.split(): x = x.replace(a,'')
        for y in x.split():
            if y not in self.C_FUNDAMENTAL: alias = y; break

        #if alias == 'class':
        #    result['class'] = result['name']    # forward decl of class
        #    result['forward_decl'] = True
        if alias == '__extension__': result['fundamental_extension'] = True
        elif alias:# and alias not in result['aliases']:
            if alias in result['aliases']: return       #print( result ); assert 0  # G3D::SkyParameters
            result['aliases'].append( alias )
            if alias in C99_NONSTANDARD:
                result['type'] = C99_NONSTANDARD[ alias ]
                result['typedef'] = alias
                result['typedefs'] += 1
            elif alias in self.typedefs:
                result['typedefs'] += 1
                result['typedef'] = alias
                self.resolve_type( self.typedefs[alias], result )

            elif 'namespace' in result and result['namespace']+alias in self.typedefs and '::' not in alias:  # is this always safe?
                alias = result['namespace']+alias
                result['typedefs'] += 1
                result['typedef'] = alias
                self.resolve_type( self.typedefs[alias], result )

            elif alias in self.classes:
                klass = self.classes[alias]; result['fundamental'] = False
                result['class'] = klass
                result['unresolved'] = False
            #else: result['unresolved'] = True
        else:
            result['fundamental'] = True
            #result['unresolved'] = False


    def finalize_vars(self):
        for s in CppStruct.Structs:    # vars within structs can be ignored if they do not resolve
            for var in s['fields']: var['parent'] = s['type']
        #for c in self.classes.values():
        #    for var in c.get_all_properties(): var['parent'] = c['name']

        ## RESOLVE ##
        for var in CppVariable.Vars:
            if 'invalid' in var and var['invalid']:		# rare cases where parser fails
                if 'method' in var: var['method']['unresolved_parameters'] = True
                if 'function' in var: var['function']['unresolved_parameters'] = True

            var['unresolved'] = False	# force False
            self.resolve_type( var['type'], var )

        # then find concrete type and best guess ctypes type #
        for var in CppVariable.Vars:    
            if not var['aliases']:    #var['fundamental']:
                var['ctypes_type'] = self.guess_ctypes_type( var['type'] )

            else:

                var['unresolved'] = False    # below may set to True
                if var['class']:
                    var['ctypes_type'] = 'ctypes.c_void_p'
                else:
                    assert var['aliases']
                    tag = var['aliases'][-1]	# get the last alias to resolve

                    klass = None
                    nestedEnum = None
                    nestedStruct = None
                    nestedTypedef = None
                    template = None
                    if 'method' in var and 'parent' in var['method']:  # if no 'parent' method of struct
                        klass = var['method']['parent']
                        if tag in klass._public_enums:
                            nestedEnum = klass._public_enums[ tag ]
                        elif tag in klass._public_structs:
                            nestedStruct = klass._public_structs[ tag ]
                        elif tag in klass._public_typedefs:
                            nestedTypedef = klass._public_typedefs[ tag ]
                        elif 'template_typename' in klass and klass['template_typename'] == tag:
                            template = tag

                    ############################
                    if template:
                        var['raw_type'] = '(template)'
                        var['template'] = template

                    elif tag in ('std::string', 'std::basic_string', 'std::basic_string<char>'):
                        var['std'] = 'string'
                        var['constant'] = True
                        var['raw_type'] = 'char'
                        var['pointer'] = 1
                        var['type'] = 'const char*'
                        var['ctypes_type'] = 'ctypes.c_char_p'
                        var['reference'] = False	# force False
                        var['fundamental'] = True

                    elif '<' in tag:    # should also contain '>'
                        #print(var)
                        T = tag.split('<')[0]#; print( self.typedefs_info.keys() )
                        typename = tag.split('<')[-1].split('>')[0]
                        Tclass = self.find_template_class( T )
                        if not Tclass:
                            var['unresolved'] = True; var['unresolved_template_class']=True; continue

                        var['raw_type'] = Tclass['namespace'] + '::' + tag.split('::')[-1]
                        var['template'] = Tclass['template_typename']; var['template_class'] = T
                        var['ctypes_type'] = 'ctypes.c_void_p'

                        Tdef = None
                        h = Tclass['namespace'] + '::' + Tclass['name']
                        if h in self.template_typedefs: Tdef = self.template_typedefs[ h ]

                        if Tclass['template_typename'] != typename:
                            if Tdef and typename in Tdef:
                                var['raw_type'] = Tdef[ typename ]['name']  # the typedef'ed name
                            else: var['unresolved'] = True
                        else:
                            var['template_requires_typename'] = True

                        var['typename_fundamental'] = is_fundamental( typename )
                        if not var['typename_fundamental']:
                            con = self.concrete_typedef( typename )
                            if con: var['template'] = con
                            else: var['template_nonfundamental'] = True


                    elif nestedEnum:
                        enum = nestedEnum
                        if enum['type'] == 'int':
                            var['ctypes_type'] = 'ctypes.c_int'
                            var['raw_type'] = 'int'

                        elif enum['type'] == 'char*':
                            var['ctypes_type'] = 'ctypes.c_char_p'
                            var['raw_type'] = 'char*'

                        var['enum'] = var['method']['path'] + '::' + enum['name']
                        var['fundamental'] = True

                    elif nestedStruct:
                        var['ctypes_type'] = 'ctypes.c_void_p'
                        var['raw_type'] = var['method']['path'] + '::' + nestedStruct['type']
                        var['fundamental'] = False
                        var['struct'] = nestedStruct['type']
                        var['nested_struct'] = True		# july17th

                    elif nestedTypedef:
                        var['nested_typedef'] = nestedTypedef
                        var['fundamental'] = is_fundamental( nestedTypedef )
                        if not var['fundamental']:
                            var['raw_type'] = var['method']['path'] + '::' + tag
                            var['ctypes_type'] = 'ctypes.c_void_p'
                        else:
                            var['raw_type'] = nestedTypedef
                            var['ctypes_type'] = self.guess_ctypes_type( nestedTypedef )

                    else:

                        _tag = tag
                        if '::' in tag and tag.split('::')[0] in self.namespaces: tag = '::'.join(tag.split('::')[1:])		#tag.split('::')[-1]
                        con = self.concrete_typedef( _tag )

                        if not con:
                            for ns in self.namespaces:
                                con = self.concrete_typedef( ns + '::' + _tag )
                                if con: break

                        if con:
                            var['concrete_type'] = _tag
                            var['type'] = con
                            self.resolve_type( var['type'], var )
                            var['ctypes_type'] = self.guess_ctypes_type( var['type'] )

                        elif tag in self.structs:
                            trace_print( 'STRUCT', var )
                            var['struct'] = tag
                            var['ctypes_type'] = 'ctypes.c_void_p'
                            var['raw_type'] = self.structs[tag]['namespace'] + '::' + tag

                        elif tag in self._forward_decls:
                            var['forward_declared'] = tag
                            var['ctypes_type'] = 'ctypes.c_void_p'

                        elif tag in self.global_enums:
                            enum = self.global_enums[ tag ]
                            if enum['type'] == 'int':
                                var['ctypes_type'] = 'ctypes.c_int'
                                var['raw_type'] = 'int'
                            elif enum['type'] == 'char*':
                                var['ctypes_type'] = 'ctypes.c_char_p'
                                var['raw_type'] = 'char*'
                            var['enum'] = enum['namespace'] + enum['name']
                            var['fundamental'] = True


                        elif var['parent']:
                            print( 'WARN unresolved', _tag)
                            var['ctypes_type'] = 'ctypes.c_void_p'
                            var['unresolved'] = True


                        elif tag.count('::')==1:
                            trace_print( 'trying to find nested something in', tag )
                            a = tag.split('::')[0]
                            b = tag.split('::')[-1]
                            if a in self.classes:    # a::b is most likely something nested in a class
                                klass = self.classes[ a ]
                                if b in klass._public_enums:
                                    trace_print( '...found nested enum', b )
                                    enum = klass._public_enums[ b ]
                                    if enum['type'] == 'int':
                                        var['ctypes_type'] = 'ctypes.c_int'
                                        var['raw_type'] = 'int'
                                    elif enum['type'] == 'char*':
                                        var['ctypes_type'] = 'ctypes.c_char_p'
                                        var['raw_type'] = 'char*'

                                    if klass['namespace']: var['enum'] = '::'.join( [klass['namespace'], klass['name'], enum['name']] )
                                    else: var['enum'] = '::'.join( [klass['name'], enum['name']] )
 
                                    var['fundamental'] = True

                                else: var['unresolved'] = True    # TODO klass._public_xxx

                            elif a in self.namespaces:    # a::b can also be a nested namespace
                                if b in self.global_enums:
                                    enum = self.global_enums[ b ]
                                    trace_print(enum)
                                trace_print(var)
                                assert 0

                            elif b in self.global_enums:        # falling back, this is a big ugly
                                enum = self.global_enums[ b ]
                                assert a in enum['namespace'].split('::')
                                if enum['type'] == 'int':
                                    var['ctypes_type'] = 'ctypes.c_int'
                                    var['raw_type'] = 'int'
                                elif enum['type'] == 'char*':
                                    var['ctypes_type'] = 'ctypes.c_char_p'
                                    var['raw_type'] = 'char*'
                                var['enum'] = enum['namespace'] + enum['name']
                                var['fundamental'] = True
                                if '::' in var['enum']:
                                    ns = var['enum'].split('::')[0]
                                    assert ns in self.namespaces
                                    if self.get_parent_namespace( ns ):
                                        var['enum'] = self.get_parent_namespace( ns ) + '::' + var['enum']

                            else:    # boost::gets::crazy
                                trace_print('NAMESPACES', self.namespaces)
                                trace_print( a, b )
                                trace_print( '---- boost gets crazy ----' )
                                var['ctypes_type'] = 'ctypes.c_void_p'
                                var['unresolved'] = True


                        elif 'namespace' in var and self.concrete_typedef(var['namespace']+tag):
                            #print( 'TRYING WITH NS', var['namespace'] )
                            con = self.concrete_typedef( var['namespace']+tag )
                            if con:
                                var['typedef'] = var['namespace']+tag
                                var['type'] = con
                                if 'struct' in con.split():
                                    var['raw_type'] = var['typedef']
                                    var['ctypes_type'] = 'ctypes.c_void_p'
                                else:
                                    self.resolve_type( var['type'], var )
                                    var['ctypes_type'] = self.guess_ctypes_type( var['type'] )

                        elif '::' in var:
                            var['ctypes_type'] = 'ctypes.c_void_p'
                            var['unresolved'] = True

                        elif tag in self.SubTypedefs:    # TODO remove SubTypedefs
                            if 'property_of_class' in var or 'property_of_struct' in var:
                                trace_print( 'class:', self.SubTypedefs[ tag ], 'tag:', tag )
                                var['typedef'] = self.SubTypedefs[ tag ]    # class name
                                var['ctypes_type'] = 'ctypes.c_void_p'
                            else:
                                trace_print( "WARN-this should almost never happen!" )
                                trace_print( var ); trace_print('-'*80)
                                var['unresolved'] = True

                        elif tag in self._template_typenames:
                            var['typename'] = tag
                            var['ctypes_type'] = 'ctypes.c_void_p'
                            var['unresolved'] = True    # TODO, how to deal with templates?

                        elif tag.startswith('_'):    # assume starting with underscore is not important for wrapping
                            print( 'WARN unresolved', _tag)
                            var['ctypes_type'] = 'ctypes.c_void_p'
                            var['unresolved'] = True

                        else:
                            trace_print( 'WARN: unknown type', var )
                            if not 'property_of_class' in var or not 'property_of_struct' in var: print('TODO - fixme', var['name'], var['type'])
                            var['unresolved'] = True


                    ## if not resolved and is a method param, not going to wrap these methods  ##
                    if var['unresolved'] and 'method' in var: var['method']['unresolved_parameters'] = True
                    if var['unresolved'] and 'function' in var: var['function']['unresolved_parameters'] = True
                    if 'template' in var and 'method' in var: var['method']['template'] = True


        # create stripped raw_type #
        p = '* & const static mutable'.split()
        for var in CppVariable.Vars:
            if 'raw_type' not in var:
                raw = []
                for x in var['type'].split():
                    if x not in p: raw.append( x )
                var['raw_type'] = ' '.join( raw )

                if var['class']:
                    if '::' not in var['raw_type']:
                        if not var['class']['parent']:
                            var['raw_type'] = var['class']['namespace'] + '::' + var['raw_type']
                        elif var['class']['parent'] in self.classes:
                            parent = self.classes[ var['class']['parent'] ]
                            var['raw_type'] = parent['namespace'] + '::' + var['class']['name'] + '::' + var['raw_type']
                        else:
                            var['unresolved'] = True

                    elif '::' in var['raw_type'] and var['raw_type'].split('::')[0] not in self.namespaces:
                        var['raw_type'] = var['class']['namespace'] + '::' + var['raw_type']
                    else:
                        var['unresolved'] = True

                elif 'forward_declared' in var and 'namespace' in var:
                    if '::' not in var['raw_type']:
                        var['raw_type'] = var['namespace'] + var['raw_type']
                    elif '::' in var['raw_type'] and var['raw_type'].split('::')[0] in self.namespaces:
                        pass
                    else: trace_print('-'*80); trace_print(var); raise NotImplemented

            ## need full name space for classes in raw type ##
            if var['raw_type'].startswith( '::' ):
                var['unresolved'] = True

            if 'method' in var and var['unresolved']: var['method']['unresolved_parameters'] = True
            if 'function' in var and var['unresolved']: var['function']['unresolved_parameters'] = True

    def concrete_typedef( self, key ):
        if key not in self.typedefs:
            #print( 'FAILED typedef', key )
            return None
        checked = []
        while key in self.typedefs and key not in checked:
            prev = key
            key = self.typedefs[ key ]
            if '<' in key or '>' in key: return prev        # stop at template
            elif key.startswith('std::'): return key        # stop at std lib
            checked.append( key )
        return key

    def get_parent_namespace(self, ns):
        for p in self.namespaces:
            if ns != p and ns in p.split('::') and p.split('::')[0] != ns:
                return p.split('::')[0]

    def find_template_class( self, T ):
        if T in self.template_classes: return self.template_classes[ T ]
        else:
            for name in self.template_classes:
                if T in name: return self.template_classes[ name ]


class _CppHeader( Resolver ):
    def finalize_return( self, meth, cls=None ):
        if meth['returns'] in 'return if else + for inline case break'.split():
            meth['returns_invalid'] = True
            return  	# TODO more irrlicht tests


        if not meth['returns_fundamental'] and meth['returns'] in C99_NONSTANDARD:
            meth['returns'] = C99_NONSTANDARD[meth['returns']]
            meth['returns_fundamental'] = True

        elif not meth['returns_fundamental']:    # describe the return type

            con = self.concrete_typedef( meth['returns'] )
            if not con and cls:
                if cls['parent'] and '::' not in meth['returns']:
                    parent = self.classes[  cls['parent']  ]
                    con = self.concrete_typedef( parent['namespace'] + '::' + meth['returns'] )
                elif cls['namespace'] and '::' not in meth['returns']:
                    con = self.concrete_typedef( cls['namespace'] + '::' + meth['returns'] )

            if not con:
                for ns in self.namespaces:
                    con = self.concrete_typedef( ns + '::' + meth['returns'] )
                    if con: break

            if con:
                if con == 'std::size_t': con = 'size_t'
                meth['returns_typedef'] = meth['returns']
                meth['returns'] = con
                meth['returns_fundamental'] = is_fundamental( con )
                if not meth['returns_fundamental']:
                    if 'typename' in con.split(): meth['returns_invalid'] = True	# TODO
                    elif con in self.classes: meth['returns_class'] = True
                    else: meth['returns_unknown'] = True

            elif meth['returns'] in self.classes:
                trace_print( 'meth returns class:', meth['returns'] )
                meth['returns_class'] = True

            elif meth['returns'] in self.SubTypedefs:
                clsname = self.SubTypedefs[ meth['returns'] ]
                meth['returns_nested'] = clsname + '::' + meth['returns']
                klass = self.classes[ clsname ]
                if meth['returns'] in klass._public_typedefs:
                    meth['returns'] = klass._public_typedefs[ meth['returns'] ]
                    if meth['returns'] in C99_NONSTANDARD: meth['returns'] = C99_NONSTANDARD[ meth['returns'] ]
                    meth['returns_fundamental'] = is_fundamental( meth['returns'] )
                    if not meth['returns_fundamental']: meth['returns_class'] = True
                else:
                    meth['returns_unsafe'] = True		# protected or private

            elif cls and meth['returns'] in cls._public_enums:
                enum = cls._public_enums[ meth['returns'] ]
                meth['returns_enum'] = enum['type']
                meth['returns_fundamental'] = True
                if enum['type'] == 'int': meth['returns'] = 'int'
                else: meth['returns'] = 'char*'

            elif meth['returns'] in self.global_enums:
                enum = self.global_enums[ meth['returns'] ]
                meth['returns_enum'] = enum['type']
                meth['returns_fundamental'] = True
                if enum['type'] == 'int': meth['returns'] = 'int'
                else: meth['returns'] = 'char*'

            elif '::' in meth['returns'] and meth['returns'].split('::')[-1] in self.global_enums:
                a = meth['returns'].split('::')[-1]
                enum = self.global_enums[ a ]
                meth['returns_enum'] = enum['type']
                meth['returns_fundamental'] = True
                if enum['type'] == 'int': meth['returns'] = 'int'
                else: meth['returns'] = 'char*'


            elif meth['returns'].count('::')==1:
                trace_print( meth )
                a,b = meth['returns'].split('::')
                if a in self.namespaces:
                    if b in self.classes:
                        klass = self.classes[ b ]
                        meth['returns_class'] = a + '::' + b

                    elif '<' in b and '>' in b:
                        meth['returns_unknown'] = True
                        meth['returns_template'] = True
                        #meth['returns_class'] = b	# template returns are always classes?

                    elif b in self.global_enums:
                        enum = self.global_enums[ b ]
                        meth['returns_enum'] = enum['type']
                        meth['returns_fundamental'] = True
                        if enum['type'] == 'int': meth['returns'] = 'int'
                        else: meth['returns'] = 'char*'

                    else: trace_print( a, b); trace_print( meth); meth['returns_unknown'] = True

                elif a in self.classes:
                    klass = self.classes[ a ]
                    if b in klass._public_enums:
                        trace_print( '...found nested enum', b )
                        enum = klass._public_enums[ b ]
                        meth['returns_enum'] = enum['type']
                        meth['returns_fundamental'] = True
                        if enum['type'] == 'int': meth['returns'] = 'int'
                        else: meth['returns'] = 'char*'

                    elif b in klass._public_forward_declares:
                        meth['returns_class'] = True

                    elif b in klass._public_typedefs:
                        meth['returns_nested_explicit'] = True
                        meth['returns_nested'] = meth['returns']
                        meth['returns'] = klass._public_typedefs[ b ]
                        meth['returns_fundamental'] = is_fundamental( meth['returns'] )
                        if not meth['returns_fundamental']: meth['returns_class'] = True

                    else:
                        trace_print( meth )    # should be a nested class, TODO fix me.
                        meth['returns_unknown'] = True

            elif '::' in meth['returns']:
                trace_print('TODO namespace or extra nested return:', meth)
                meth['returns_unknown'] = True

            elif self.concrete_typedef( meth['namespace'].split('::')[0] + '::' + meth['returns'] ):
                con = self.concrete_typedef( meth['namespace'].split('::')[0] + '::' + meth['returns'] )
                if con == 'std::size_t': con = 'size_t'
                meth['returns_typedef'] = meth['returns']
                meth['returns'] = con
                meth['returns_fundamental'] = is_fundamental( con )
                if not meth['returns_fundamental']:
                    if 'typename' in con.split(): meth['returns_invalid'] = True	# TODO
                    elif con in self.classes: meth['returns_class'] = True
                    else: meth['returns_unknown'] = True


            else:
                trace_print( 'WARN: UNKNOWN RETURN', meth['name'], meth['returns'])
                #print( self.global_enums.keys() )
                meth['returns_unknown'] = True

        if meth['returns_fundamental']: meth['returns_ctypes'] = self.guess_ctypes_type( meth['returns'] )

    def finalize(self):
        ## finalize templates ##
        for typedef in self.typedefs_info:
            tdef = self.typedefs_info[ typedef ]
            if 'template' in tdef:
                T = self.template_typedefs[ tdef['template'] ]
                if tdef['template'] not in self.template_classes: continue		# TODO fix me

                cls = self.template_classes[ tdef['template'] ]
                for info in T.values():
                    if ',' in info['typename']: continue		# TODO - complex templates

                    info['fundamental'] = is_fundamental( info['typename'] )

                    if not info['fundamental']:
                        tn = info['typename']
                        con = self.concrete_typedef( tn )
                        if not con and cls:
                            if cls['parent'] and '::' not in tn:
                                parent = self.classes[  cls['parent']  ]
                                con = self.concrete_typedef( parent['namespace'] + '::' + tn )
                            elif cls['namespace'] and '::' not in tn:
                                con = self.concrete_typedef( cls['namespace'] + '::' + tn )

                        if not con:
                            for ns in self.namespaces:
                                #print('trying', ns + '::' + tn)
                                con = self.concrete_typedef( ns + '::' + tn )
                                if con: break
                        if con:
                            if con == 'std::size_t': con = 'size_t'
                            info['type'] = info['type'].replace('<%s>'%tn, '<%s>'%con)
                            info['fundamental'] = is_fundamental( con )
                            info['alias'] = tn
                            info['typename'] = con
                            info['ctypes_type'] = self.guess_ctypes_type( con )
                            cls['template_info'] = T		# do not resolve structs now

                        elif tn in self.structs:
                            info['fundamental'] = False
                            info['struct'] = True
                            info['ctypes_type'] = 'ctypes.c_void_p'
                        elif '::' in tn and tn.split('::')[-1] in self.structs:
                            info['fundamental'] = False
                            info['struct'] = True
                            info['ctypes_type'] = 'ctypes.c_void_p'

                        else: print('TODO template', tn)


        ## finalize variables ##
        self.finalize_vars()

        # finalize method returns types
        for cls in self.classes.values():
            for meth in cls.get_all_methods():
                if meth['pure_virtual']: cls['abstract'] = True
                self.finalize_return( meth, cls )

        ## finalize free function returns ##
        for func in self.functions: self.finalize_return( func )


        ## find pure virtuals and mark abstract classes ##
        for cls in self.classes.values():
            methnames = cls.get_all_method_names()
            pvm = cls.get_all_pure_virtual_methods()

            for d in cls['inherits']:
                c = d['class']
                a = d['access']    # do not depend on this to be 'public'
                trace_print( 'PARENT CLASS:', c )
                if c not in self.classes: trace_print('WARN: parent class not found')
                if c in self.classes and self.classes[c]['abstract']:
                    p = self.classes[ c ]
                    for meth in p.get_all_methods():    #p["methods"]["public"]:
                        trace_print( '\t\tmeth', meth['name'], 'pure virtual', meth['pure_virtual'] )
                        if meth['pure_virtual'] and meth['name'] not in methnames: cls['abstract'] = True; break

        print self.typedefs.keys()
        print '-------------'
        for k in self.template_typedefs.keys(): print k
        #assert 'irr::core::dimension2d<u32>' in self.typedefs
        #assert 0




    def evaluate_struct_stack(self):
        """Create a Struct out of the name stack (but not its parts)"""
        #print( 'eval struct stack', self.nameStack )
        #if self.braceDepth != len(self.nameSpaces): return
        struct = CppStruct(self.nameStack)
        struct["namespace"] = self.cur_namespace()
        self.structs_order.append( struct )
        parentStruct = self.curStruct
        self.curStruct = struct
        if not struct['type']: return	# struct {} name;

        if self.curClass:
            struct['parent'] = self.curClass
            klass = self.classes[ self.curClass ]
            klass['structs'][self.curAccessSpecifier].append( struct )
            if self.curAccessSpecifier == 'public': klass._public_structs[ struct['type'] ] = struct
            self.structs[ self.curClass + '::' + struct['type'] ] = struct

        elif parentStruct:
            print( 'TODO - struct in struct' )
            struct['nested'] = True
            self.structs[ struct['type'] ] = struct

        else:
            self.structs[ struct['type'] ] = struct

        self._structs_brace_level[ struct['type'] ] = self.braceDepth

    ## python style ##
    PYTHON_OPERATOR_MAP = {
        '()' : '__call__',
        '[]' : '__getitem__',
        '<'    :    '__lt__',
        '<='    :    '__le__',
        '=='    :    '__eq__',
        '!='    :    '__ne__',
        '>'    :    '__gt__',
        '>='    :    '__ge__',
        '+'    :    '__add__',
        '-'    :    '__sub__',
        '*'    :    '__mul__',
        '%'    :    '__divmod__',
        '**'    :    '__pow__',
        '>>'    :    '__lshift__',
        '<<'    :    '__rshift__',
        '&'    :    '__and__',
        '^'    :    '__xor__',
        '|'    :    '__or__',
        '+='    :    '__iadd__',
        '-='    :    '__isub__',
        '*='    :    '__imult__',
        '/='    :    '__idiv__',
        '%='    :    '__imod__',
        '**='    :    '__ipow__',
        '<<='    :    '__ilshift__',
        '>>='    :    '__irshift__',
        '&='    :    '__iand__',
        '^='    :    '__ixor__',
        '|='    :    '__ior__',
        #__neg__ __pos__ __abs__; what are these in c++?
        '~'    :    '__invert__',
        '.'    :    '__getattr__',
    }
    OPERATOR_MAP = {    # do not use double under-scores so no conflicts with python #
        '='    :    '_assignment_',
        '->'    :    '_select_member_',
        '++'    :    '_increment_',
        '--'    :    '_deincrement_',
        'new'    :    '_new_',
        'delete' :    '_delete_',
    }
    OPERATOR_MAP.update( PYTHON_OPERATOR_MAP )

    def parse_method_type( self, stack ):
        #print( 'meth type info', stack )
        if stack[0] in ':;': stack = stack[1:]
        info = { 
            'debug': ' '.join(stack), 
            'class':None, 
            'namespace':self.current_namespace(),
        }
        if stack[0] == 'extern':
            info['extern'] = True
            if stack[1] == '"C"': stack = stack[ 2 : ]; info['C'] = True
            elif stack[1] == '"C++"': stack = stack[ 2 : ]
            else: stack = stack[ 1 : ]

        for tag in 'defined pure_virtual operator constructor destructor extern template virtual static explicit inline friend returns returns_pointer returns_fundamental returns_class'.split(): info[tag]=False

        for a in ('__attribute__', '__attribute', 'throw'):
            if a in stack:
                x = []; hit=None; skip=0
                for i,b in enumerate(stack):
                    if b==a: hit = i
                    elif hit is not None and b=='(' and i-1 == hit:
                        skip = 1
                    elif hit and b=='(':
                        skip += 1
                    elif skip and b==')':
                        skip -= 1
                        if skip == 0: hit = None
                    elif not hit and not skip:
                        x.append( b )
                if x: stack = x
                while stack and stack[0]==')': stack.pop(0)
                # is_method can be confused by properties like: "__pthread_unwind_buf_t __attribute__((__aligned__));"
                # and ends up passing to here thinking its a method.
                if '(' not in stack: return None
                #print('AFTER------',stack)

        assert stack.count('(') == stack.count(')')

        header = stack[ : stack.index('(') ]
        header = ' '.join( header )
        header = header.replace(' : : ', '::' )
        header = header.replace(' < ', '<' )
        header = header.replace(' > ', '> ' )
        header = header.strip()

        if '{' in stack:
            info['defined'] = True
            self._method_body = self.braceDepth
            print( '----------NEW METHOD WITH BODY---------', self.braceDepth )
        elif stack[-1] == ';':
            info['defined'] = False
            self._method_body = None    # this is force cleared in several other places
        else: assert 0

        if len(stack) > 3 and stack[-1] == ';' and stack[-2] == '0' and stack[-3] == '=':
            info['pure_virtual'] = True
        elif len(stack) > 2 and stack[-1] == ';' and stack[-2] == '=0':
            info['pure_virtual'] = True
        elif stack[-1] == '=0;':
            info['pure_virtual'] = True

        r = header.split()
        name = None
        if 'operator' in stack:    # rare case op overload defined outside of class
            op = stack[ stack.index('operator')+1 : stack.index('(') ]
            op = ''.join(op)
            if not op:
                trace_print( 'TODO - parse () operator' )
                return None
            else:
                info['operator'] = op
                if op in self.OPERATOR_MAP:
                    name = '__operator__' + self.OPERATOR_MAP[ op ]
                    a = stack[ : stack.index('operator') ]
                else:
                    trace_print('ERROR - not a C++ operator', op)
                    return None

        elif r:        # June 23 2011
            name = r[-1]
            a = r[ : -1 ]    # strip name

        if name is None: print('HEAD', header); assert 0 #return None

        while a and a[0] == '}':    # strip - can have multiple } }
            a = a[1:]    # july3rd

        print(name)
        if '::' in name:
            klass = name[ : name.rindex('::') ]
            name = name.split('::')[-1]
            info['class'] = klass

        if name.startswith('~'):
            info['destructor'] = True
            name = name[1:]
        elif not a:
            info['constructor'] = True

        info['name'] = name

        for tag in 'virtual static friend explicit inline __explicit__ __inline__'.split():
            if tag in a: info[ tag.replace('_','') ] = True; a.remove( tag )    # inplace


        if 'template' in a:
            a.remove('template')
            b = ' '.join( a )
            if '>' in b:
                info['template'] = b[ : b.index('>')+1 ]
                info['returns'] = b[ b.index('>')+1 : ]    # find return type, could be incorrect... TODO
                if '<typename' in info['template'].split():
                    typname = info['template'].split()[-1]
                    typname = typname[ : -1 ]    # strip '>'
                    if typname not in self._template_typenames: self._template_typenames.append( typname )
            else: info['returns'] = ' '.join( a )
        else: info['returns'] = ' '.join( a )

        info['returns'] = info['returns'].replace(" <","<")
        info['returns'] = info['returns'].replace(" >",">")
        info['returns'] = info['returns'].strip()

        ## be careful with templates, do not count pointers inside template
        info['returns_pointer'] = info['returns'].split('>')[-1].count('*')
        if info['returns_pointer']: info['returns'] = info['returns'].replace('*','').strip()

        info['returns_reference'] = '&' in info['returns']
        if info['returns']: info['returns'] = info['returns'].replace('&','').strip()

        a = []
        for b in info['returns'].split():
            if b == '__const__': info['returns_const'] = True
            elif b == 'const': info['returns_const'] = True
            else: a.append( b )
        info['returns'] = ' '.join( a )

        info['returns_fundamental'] = is_fundamental( info['returns'] )
        return info

    def evaluate_method_stack(self):
        """Create a method out of the name stack"""

        stack = []      ## fixes "/("
        for a in self.stack:
            if a == '/(': stack.append( '/' ); stack.append('(')
            else: stack.append( a )
        info = self.parse_method_type( stack )

        if info:
            if info[ 'class' ] and info['class'] in self.classes:     # case where methods are defined outside of class
                klass = self.classes[ info['class'] ]
                if not info['name'] in klass.get_all_method_names():    # do not overwrite header defined method
                    print('External Method:', info['name'])
                    newMethod = CppMethod(self.nameStack, info['name'], info)
                    klass[ 'methods' ][ 'public' ].append( newMethod )    # it could be private, how do we know this.
                    newMethod['parent'] = klass
                    if klass['namespace']: newMethod['path'] = klass['namespace'] + '::' + klass['name']
                    else: newMethod['path'] = klass['name']
            elif info['class']: pass
            elif '::' in info['name']: print( info ); assert 0

            elif self.curStruct:	# struct is a class in c++
                print( 'Struct Method:', info )
                newMethod = CppMethod(self.nameStack, self.curStruct, info)
                self.curStruct['methods'].append( newMethod )

            elif self.curClass:    # normal case
                trace_print( 'Internal Method:', info )
                newMethod = CppMethod(self.nameStack, self.curClass, info)
                klass = self.classes[self.curClass]
                klass['methods'][self.curAccessSpecifier].append(newMethod)
                newMethod['parent'] = klass
                if klass['namespace']: newMethod['path'] = klass['namespace'] + '::' + klass['name']
                else: newMethod['path'] = klass['name']


            else:
                print( 'free function:', self.nameStack )
                if 'operator' in info and info['operator']:
                    trace_print( 'skipping external method def with operator', info )
                else:
                    func = CppMethod(self.nameStack, methinfo=info)
                    self.functions.append( func )
                    if func['name'] == 'setNull': print(info); assert 0

        self.stack = []

    def _parse_typedef( self, stack, namespace='' ):
        if not stack or 'typedef' not in stack: return
        stack = list( stack )    # copy just to be safe
        if stack[-1] == ';': stack.pop()

        while stack and stack[-1].isdigit(): stack.pop()    # throw away array size for now

        idx = stack.index('typedef')
        name = namespace + stack[-1]
        s = ''
        for a in stack[idx+1:-1]:
            if a == '{': break
            if not s or s[-1] in ':<>' or a in ':<>': s += a    # keep compact
            else: s += ' ' + a    # spacing
        s = s.replace(' <', '<')
        r = {'name':name, 'raw':s, 'type':s}
        if not is_fundamental(s):
            if 'struct' in s.split(): pass        # TODO is this right? "struct ns::something"
            elif '::' not in s: s = namespace + s         # only add the current name space if no namespace given
            #elif '::' in s:
            #    ns = s.split('::')[0]
            #    if ns not in self.namespaces:
            r['type'] = s
            if '<' in s:
                r['template'] = s.split('<')[0]
                r['typename'] = s.split('<')[-1].split('>')[0]
        if s: return r


    def evaluate_typedef(self):
        ns = self.cur_namespace(add_double_colon=True)
        res = self._parse_typedef( self.stack, ns )
        if res:
            name = res['name']
            self.typedefs[ name ] = res['type']
            self.typedefs_info[ name ] = res
            if name not in self.typedefs_order: self.typedefs_order.append( name )
            if 'template' in res:
                T = res['template']
                if T not in self.template_typedefs: self.template_typedefs[ T ] = {}
                self.template_typedefs[ T ][ res['typename'] ] = res

    def evaluate_property_stack(self):
        """Create a Property out of the name stack"""
        assert self.stack[-1] == ';'
        if self.nameStack[0] == 'typedef':
            if self.curClass:
                typedef = self._parse_typedef( self.stack )
                name = typedef['name']
                klass = self.classes[ self.curClass ]
                klass[ 'typedefs' ][ self.curAccessSpecifier ].append( name )
                if self.curAccessSpecifier == 'public': klass._public_typedefs[ name ] = typedef['type']
                Resolver.SubTypedefs[ name ] = self.curClass
            else: assert 0
        elif self.curStruct or self.curClass:
            newVar = CppVariable(self.nameStack, self.stack)
            newVar['namespace'] = self.current_namespace()
            if self.curStruct:
                self.curStruct[ 'fields' ].append( newVar )
                newVar['property_of_struct'] = self.curStruct
            elif self.curClass:
                klass = self.classes[self.curClass]
                klass["properties"][self.curAccessSpecifier].append(newVar)
                newVar['property_of_class'] = klass['name']

        self.stack = []        # CLEAR STACK

    def evaluate_class_stack(self):
        """Create a Class out of the name stack (but not its parts)"""
        parent = self.curClass
        print('NEWCLASS', ' '.join(self.nameStack))

        if self.braceDepth > len( self.nameSpaces) and (parent or self.curStruct):
            print( 'HIT NESTED SUBCLASS' )
        elif self.braceDepth-1 != len(self.nameSpaces):
            print( 'ERROR: WRONG BRACE DEPTH', self.braceDepth, self.nameSpaces )
            assert 0

        self._method_body = None	# force clear

        prevAccess = None
        if parent: prevAccess = self.curAccessSpecifier
        self.curAccessSpecifier = 'private'        # private is default
        self._current_access.append( self.curAccessSpecifier )

        newClass = CppClass(self.nameStack)
        if 'name' not in newClass or not newClass['name']: assert 0
        print( 'CLASS OK', newClass['name'] )
        if newClass['name'] in 'RenderQueueInvocationIterator RenderQueueInvocationList'.split(): assert 0

        if '>' in newClass['name']: print('WARN: strange template class', newClass['name'])

        self.classes_order.append( newClass )    # good idea to save ordering
        self.stack = []        # fixes if class declared with ';' in closing brace

        if parent:
            newClass["namespace"] = self.classes[ parent ]['namespace'] + '::' + parent
            newClass['parent'] = parent
            self.classes[ parent ]['nested_classes'].append( newClass )
            ## supports nested classes with the same name ##
            self.curClass = key = parent+'::'+newClass['name']
            self._classes_brace_level[ key ] = self.braceDepth
            ## nested classes that are protected or private can not be exposed in a C wrapper ##
            if prevAccess and prevAccess in ('protected', 'private'): newClass['internal'] = True
            elif 'internal' in self.classes[parent]: newClass['internal'] = True
            elif 'template_typename' in self.classes[parent]: newClass['internal'] = True; newClass['nested_in_template'] = True

        elif newClass['parent']:        # nested class defined outside of parent.  A::B {...}
            parent = newClass['parent']
            newClass["namespace"] = self.classes[ parent ]['namespace'] + '::' + parent
            self.classes[ parent ]['nested_classes'].append( newClass )
            ## supports nested classes with the same name ##
            self.curClass = key = parent+'::'+newClass['name']
            self._classes_brace_level[ key ] = self.braceDepth
            if 'internal' in self.classes[parent]: newClass['internal'] = True
            elif 'template_typename' in self.classes[parent]: newClass['internal'] = True; newClass['nested_in_template'] = True

        else:
            newClass["namespace"] = self.cur_namespace()
            key = newClass['name']
            self.curClass = newClass["name"]
            self._classes_brace_level[ newClass['name'] ] = self.braceDepth

        if newClass['namespace'] in ('std', 'boost'):
            self.curClass = key = newClass['namespace'] + '::' + key
            self._classes_brace_level[ key ] = self.braceDepth

        if key in self.classes:
            print('WARN - name collision', key)

        self.classes[ key ] = newClass
        if 'template_typename' in newClass:
            self.template_classes[ self.current_namespace() + key ] = newClass
            if newClass['name'] not in self.template_classes:
                self.template_classes[ newClass['name'] ] = newClass


    def evaluate_forward_decl(self):
        trace_print( 'FORWARD DECL', self.nameStack )
        #assert self.nameStack[0] == 'class'
        name = self.nameStack[-1]
        if self.curClass:
            klass = self.classes[ self.curClass ]
            klass['forward_declares'][self.curAccessSpecifier].append( name )
            if self.curAccessSpecifier == 'public': klass._public_forward_declares.append( name )
        else: self._forward_decls.append( name )

class CppHeader( _CppHeader ):
    """Parsed C++ class header
    
    Variables produced:
    self.classes - Dictionary of classes found in a given header file where the
        key is the name of the class
    """
    IGNORE_NAMES = '__extension__'.split()
    
    def show(self):
        for className in self.classes.keys(): self.classes[className].show()

    
    def evaluate_enum_stack(self):
        """Create an Enum out of the name stack"""
        newEnum = CppEnum(self.nameStack)
        if len(newEnum.keys()):
            if len(self.curClass):
                newEnum["namespace"] = self.cur_namespace(False)
                klass = self.classes[self.curClass]
                klass["enums"][self.curAccessSpecifier].append(newEnum)
                if self.curAccessSpecifier == 'public':
                    if 'name' in newEnum and newEnum['name']:
                        klass._public_enums[ newEnum['name'] ] = newEnum
            else:
                newEnum["namespace"] = self.cur_namespace(True)
                self.enums.append(newEnum)
                if 'name' in newEnum and newEnum['name']: self.global_enums[ newEnum['name'] ] = newEnum

            #This enum has instances, turn them into properties
            if newEnum.has_key("instances"):
                instanceType = "enum"
                if newEnum.has_key("name"):
                    instanceType = newEnum["name"]
                for instance in newEnum["instances"]:
                    self.nameStack = [instanceType,  instance]
                    self.evaluate_property_stack()
                del newEnum["instances"]


    def __init__(self, headerFileName, argType="file", **kwargs):
        """Create the parsed C++ header file parse tree
        
        headerFileName - Name of the file to parse OR actual file contents (depends on argType)
        argType - Indicates how to interpret headerFileName as a file string or file name
        kwargs - Supports the following keywords
         "enumMaintianValueFormat" - Set to true for enum values to maintain the original format ('j' will not convert to 106)
        """
        ## reset global state ##
        global doxygenCommentCache
        doxygenCommentCache = ""
        CppVariable.Vars = []
        CppStruct.Structs = []

        if (argType == "file"):
            self.headerFileName = os.path.expandvars(headerFileName)
            self.mainClass = os.path.split(self.headerFileName)[1][:-2]
            headerFileStr = ""
        elif argType == "string":
            self.headerFileName = ""
            self.mainClass = "???"
            headerFileStr = headerFileName
        else:
            raise Exception("Arg type must be either file or string")
        self.curClass = ""
        
        global enumMaintianValueFormat
        if kwargs.has_key("enumMaintianValueFormat"):
            enumMaintianValueFormat = kwargs["enumMaintianValueFormat"]
        else:
            enumMaintianValueFormat = False

        # nested classes have parent::nested, but no extra namespace,
        # this keeps the API compatible, TODO proper namespace for everything. 
        Resolver.CLASSES = {}
        self.classes = Resolver.CLASSES

        self.enums = []
        self.global_enums = {}
        self.nameStack = []
        self.nameSpaces = []
        self.curAccessSpecifier = 'private'    # private is default
        self._current_access = []
        self.initextra()    # harts hack
    
        if (len(self.headerFileName)):
            headerFileStr = "\n".join(open(self.headerFileName).readlines())
        self.braceDepth = 0
        lex.input(headerFileStr)
        curLine = 0
        curChar = 0
        if 1:    #try:
            while True:
                tok = lex.token()
                if not tok: break
                if tok.type == 'NAME' and tok.value in self.IGNORE_NAMES: continue
                if tok.type not in ('PRECOMP_MACRO', 'PRECOMP_MACRO_CONT'): self.stack.append( tok.value )
                curLine = tok.lineno
                curChar = tok.lexpos

                if tok.type in ('OPEN_BRACKET', 'CLOSE_BRACKET'): self.nameStack.append( tok.value )

                elif (tok.type == 'OPEN_BRACE'):
                    _brace = True
                    if len(self.nameStack)>=2 and self.nameStack[0]=='extern' and self.nameStack[1]=='"C"':
                        _brace = False; print( 'extern C')
                    elif len(self.nameStack)>=2 and self.nameStack[0]=='extern' and self.nameStack[1]=='"C++"':
                        _brace = False; print( 'extern C++' )
                    if _brace: self.braceDepth += 1


                    if len(self.nameStack) >= 2 and is_namespace(self.nameStack):    # namespace {} with no name used in boost, this sets default?
                        self.nameSpaces.append(self.nameStack[1])
                        ns = self.cur_namespace(); self.stack = []
                        if ns not in self.namespaces: self.namespaces.append( ns )
                    if len(self.nameStack) and not is_enum_namestack(self.nameStack):
                        self.evaluate_stack()
                    else:
                        self.nameStack.append(tok.value)
                    if self.stack and self.stack[0] == 'class': self.stack = []

                    #if _brace: self.braceDepth += 1

                elif (tok.type == 'CLOSE_BRACE'):

                    if self.braceDepth == 0:
                        continue
                    if (self.braceDepth == len(self.nameSpaces)):
                        tmp = self.nameSpaces.pop()
                        self.stack = []    # clear stack when namespace ends?
                    if len(self.nameStack) and is_enum_namestack(self.nameStack):
                        self.nameStack.append(tok.value)
                    elif self.braceDepth < 10:
                        self.evaluate_stack()
                    else:
                        self.nameStack = []
                    self.braceDepth -= 1

                    if self.braceDepth < 0:
                        print('---------- END OF EXTERN -----------')
                        self.braceDepth = 0

                    if self.curClass and debug: print( 'CURBD', self._classes_brace_level[ self.curClass ] )

                    if (self.braceDepth == 0) or (self.curClass and self._classes_brace_level[self.curClass] > self.braceDepth):
                        if self.curClass: print( '------------END OF CLASS DEF-------------', 'braceDepth:', self.braceDepth )

                        if self._current_access: self._current_access.pop()

                        if self.curClass and self.classes[ self.curClass ]['parent']:
                            self.curClass = self.classes[ self.curClass ]['parent']
                            if self._current_access: self.curAccessSpecifier = self._current_access[-1]
                        else:
                            self.curClass = ""
                        self.stack = []

                    #if self.curStruct: self.curStruct = None
                    if self.braceDepth==0 or (self.curStruct and not self.curStruct['type']) or (self.curStruct and self._structs_brace_level[self.curStruct['type']] > self.braceDepth):
                        if self.curStruct: print( '---------END OF STRUCT DEF-------------' )
                        if self.curStruct and not self.curStruct['type']: self._struct_needs_name = self.curStruct
                        self.curStruct = None

                    if self._method_body and self.braceDepth < self._method_body:
                        self._method_body = None; self.stack = []; self.nameStack = []; print( 'FORCE CLEAR METHBODY' )
                
                if (tok.type == 'OPEN_PAREN'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'CLOSE_PAREN'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'EQUALS'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'COMMA'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'NUMBER'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'MINUS'):
                    self.nameStack.append(tok.value)
                elif (tok.type == 'PLUS'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'STRING_LITERAL'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'NAME' or tok.type == 'AMPERSTAND' or tok.type == 'ASTERISK'):
                    self.nameStack.append(tok.value)

                elif (tok.type == 'COLON'):
                    #Dont want colon to be first in stack
                    if len(self.nameStack) == 0:
                        continue
                    if self.nameStack and self.nameStack[-1] in supportedAccessSpecifier:
                        if self.curClass or self.curStruct:
                            cas = self.nameStack[-1]
                            self.curAccessSpecifier = cas; print('CURACCESS-set', cas)
                            if self.curClass:
                                if self._current_access: self._current_access[-1] = cas
                                else: self._current_access.append( cas )
                        else: print('warning - "public ::namespace"', ' '.join(self.nameStack))

                        self.stack = []; self.nameStack = []	# need to clear nameStack to so that nested structs can be found
                    else:
                        self.nameStack.append(tok.value)

                elif (tok.type == 'SEMI_COLON'):
                    if (self.braceDepth < 10): self.evaluate_stack( tok.type )
                    if not self.stack: continue
                    if self.stack[0]=='typedef' and ( '{' not in self.stack or '}' in self.stack ): self.stack = []; trace_print( "REAL CLEAR")
                    elif self.stack[0] != 'typedef': self.stack = []; trace_print('CLEAR STACK')

        #except:
        #    raise CppParseError("Not able to parse %s on line %d evaluating \"%s\"\nError around: %s"
        #                        % (self.headerFileName, tok.lineno, tok.value, " ".join(self.nameStack)))

        self.finalize()

    def evaluate_stack(self, token=None):
        """Evaluates the current name stack"""
        global doxygenCommentCache
        print( "Evaluating stack %s\nBraceDepth: %s" %(self.nameStack,self.braceDepth))
        print( "Evaluating stack %s\nBraceDepth: %s" %(self.stack,self.braceDepth))
        if (len(self.curClass)):
            if (debug): print( "%s (%s) "%(self.curClass, self.curAccessSpecifier))

        #if 'typedef' in self.nameStack: self.evaluate_typedef()        # allows nested typedefs, probably a bad idea
        if not self.curClass and 'typedef' in self.nameStack:
            print('HIT TYPEDEF', self.stack)
            if token == 'SEMI_COLON' and ('{' not in self.stack or '}' in self.stack): self.evaluate_typedef()
            else: return

        elif (len(self.nameStack) == 0):
            if (debug): print( "line ",lineno() )
            if (debug): print( "(Empty Stack)" )
            return
        elif (self.nameStack[0] == "namespace"):
            #Taken care of outside of here
            pass
        elif len(self.nameStack) >= 2 and self.nameStack[0] == 'using' and self.nameStack[1] == 'namespace': pass    # TODO

        elif is_enum_namestack(self.nameStack):
            if (debug): print( "line ",lineno() )
            self.evaluate_enum_stack()

        elif self._method_body and self.braceDepth >= self._method_body:
            #print( 'INSIDE METHOD DEF', self.nameStack )
            self.stack = []

        #elif is_method_namestack(self.stack) and '(' in self.nameStack:    # this fails on "operator /(..."
        elif ')' in self.nameStack and is_method_namestack(self.stack):
            #print( 'eval method', self.nameStack )
            self.evaluate_method_stack()
            self.stack = []

        elif len(self.nameStack) >= 2 and (self.nameStack[0]=='friend' and self.nameStack[1]=='class'): pass

        elif ('class' in self.nameStack or 'struct' in self.nameStack) and self.stack[-1] == ';': self.evaluate_forward_decl()

        elif (self.nameStack[0] == "class") or (self.nameStack[0]=='template' and 'class' in self.nameStack):
            #print('^^^^^^^^^^^^^^^^^^^^')
            self.evaluate_class_stack()
        elif (self.nameStack[0] == "struct") or (len(self.nameStack)>3 and self.stack[-1]=='{' and self.nameStack[-3]=='struct'):
            print( '------------new struct-----------' )
            self.evaluate_struct_stack()
            self.stack = []
        elif self.nameStack[0]=='template' and self.stack[-1]=='{' and 'struct' in self.nameStack:
            print( '------------new struct - unsafe?' )
            self.evaluate_struct_stack()
            self.stack = []

        elif '(' not in self.nameStack and ')' not in self.nameStack and self.stack[-1] == ';':	# catching all props?
            self.evaluate_property_stack()


        elif not self.curClass:
            if (debug): print( "line ",lineno() )
            if is_enum_namestack(self.nameStack): self.evaluate_enum_stack()
            elif self.curStruct and self.stack[-1] == ';': self.evaluate_property_stack()    # this catches fields of global structs
            self.nameStack = []
            doxygenCommentCache = ""
            return
        elif (self.braceDepth < 1):
            if (debug): print( "line ",lineno() )
            #Ignore global stuff for now
            if (debug): print( "Global stuff: ",  self.nameStack )
            self.nameStack = []
            self._method_body = None
            doxygenCommentCache = ""
            return
        elif (self.braceDepth > len(self.nameSpaces) + 1):
            if (debug): print( "line ",lineno() )
            self.nameStack = []
            doxygenCommentCache = ""
            return

        self.nameStack = []        # some if/else above return and others not, so this may or may not be reset
        doxygenCommentCache = ""



