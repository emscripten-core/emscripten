#!/usr/bin/python
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
CppHeaderParser2.0: April 2011 - July 2011
	by HartsAntler
	bhartsho@yahoo.com
	http://pyppet.blogspot.com

	Quick Start - User API:
		h = CppHeaderParser.CppHeader("someheader.h")
		for name in h.classes:
			c = h.classes[name]
			for method in c['methods']['public']:
				print method['name']
				print dir(method)		# view the rest of the API here.

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


	Internal Developer Notes:

		1. in method['rtnType'] is not recommended, instead use: 'returns', 'returns_pointer', 'returns_reference', 'returns_fundamental'
			(camelCase may become deprecated in the future for the dict lookup keys)

		2. double name stacks:
			. the main stack is self.nameStack, this stack is simpler and easy to get hints from
			. the secondary stack is self.stack is the full name stack, required for parsing somethings
			. each stack maybe cleared at different points, since they are used to detect different things
			. it looks ugly but it works :)

		3. Tabs vs Spaces:
			This file now contains tabs and spaces, the tabb'ed code is the new stuff,
			in the future this should be made consistent, one way or the other.

		4. Had to make the __repr__ methods simple because some of these dicts are interlinked.
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


version = __version__ = "1.9.9"

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
]

t_ignore = " \t\r[].|!?%@'^\\"		# harts hack (litteral backslash is a bad idea?) - old version: " \t\r[].|!?%@"
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
t_DIVIDE = r'/[^/]'
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

supportedAccessSpecifier = [
    'public',
    'protected', 
    'private'
]

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
    
class CppClass(dict):
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
        for typ in 'public protected private'.split(): r += self.get_method_names(typ)		# returns list
        return r

    def get_all_pure_virtual_methods( self ):
        r = {}
        for typ in 'public protected private'.split(): r.update(self.get_pure_virtual_methods(typ))		# returns dict
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
        self._public_enums = {}
        self._public_structs = {}
        self._public_typedefs = {}
        self._public_forward_declares = []
        self['namespace'] = ""

        if (debug): print( "Class:   ",  nameStack )
        if (len(nameStack) < 2):
            print( "Error detecting class" )
            return
        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""
        self["name"] = nameStack[1]
        inheritList = []

        if ":" in nameStack:
            self['name'] = nameStack[ nameStack.index(':') - 1 ]	# harts hack - fixes:  class __something__ classname

        if nameStack.count(':') == 1:
            nameStack = nameStack[nameStack.index(":") + 1:]
            while len(nameStack):
                tmpStack = []
                tmpInheritClass = {"access":"private"}	# shouldn't public be default?
                if "," in nameStack:
                    tmpStack = nameStack[:nameStack.index(",")]
                    nameStack = nameStack[nameStack.index(",") + 1:]
                else:
                    tmpStack = nameStack
                    nameStack = []
                if len(tmpStack) == 0:
                    break;
                elif len(tmpStack) == 1:
                    tmpInheritClass["class"] = tmpStack[0]
                elif len(tmpStack) == 2:
                    tmpInheritClass["access"] = tmpStack[0]
                    tmpInheritClass["class"] = tmpStack[1]
                else:
                    print( "Warning: can not parse inheriting class %s"%(" ".join(tmpStack)))
                    if '>' in tmpStack: pass	# allow skip templates for now
                    else: raise NotImplemented

                if 'class' in tmpInheritClass: inheritList.append(tmpInheritClass)

        elif nameStack.count(':') == 2: self['parent'] = self['name']; self['name'] = nameStack[-1]

        elif nameStack.count(':') > 2: print('ERROR can not parse', nameStack)	# TODO

        self['inherits'] = inheritList

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

 
    def show(self):
        """Convert class to a string"""
        namespace_prefix = ""
        if self["namespace"]: namespace_prefix = self["namespace"] + "::"
        rtn = "class %s"%(namespace_prefix + self["name"])
        if self['abstract']: rtn += '	(abstract)\n'
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
            rtn += "	%s\n"%(accessSpecifier)
            #Enums
            if (len(self["enums"][accessSpecifier])):
                rtn += "		<Enums>\n"
            for enum in self["enums"][accessSpecifier]:
                rtn += "			%s\n"%(repr(enum))
            #Properties
            if (len(self["properties"][accessSpecifier])):
                rtn += "		<Properties>\n"
            for property in self["properties"][accessSpecifier]:
                rtn += "			%s\n"%(repr(property))
            #Methods
            if (len(self["methods"][accessSpecifier])):
                rtn += "		<Methods>\n"
            for method in self["methods"][accessSpecifier]:
                rtn += "\t\t" + method.show() + '\n'
        rtn += "  }\n"
        print( rtn )

class _CppMethod( dict ):
	def _params_helper( self, params ):
		for p in params:
			p['method'] = self		# save reference in variable to parent method
			if '::' in p['type']:
				ns = p['type'].split('::')[0]
				if ns not in Resolver.NAMESPACES and ns in Resolver.CLASSES:
					p['type'] = self['namespace'] + p['type']
			else: p['namespace'] = self[ 'namespace' ]

class CppMethod( _CppMethod ):
    """Takes a name stack and turns it into a method
    
    Contains the following Keys:
    self['rtnType'] - Return type of the method (ex. "int")
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

    def __init__(self, nameStack, curClass, methinfo):
        if (debug): print( "Method:   ",  nameStack )
        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""
        if "operator" in nameStack:
            self["rtnType"] = " ".join(nameStack[:nameStack.index('operator')])
            self["name"] = "".join(nameStack[nameStack.index('operator'):nameStack.index('(')])
        else:
            self["rtnType"] = " ".join(nameStack[:nameStack.index('(') - 1])
            self["name"] = " ".join(nameStack[nameStack.index('(') - 1:nameStack.index('(')])
        if len(self["rtnType"]) == 0 or self["name"] == curClass:
            self["rtnType"] = "void"

        self.update( methinfo )	# harts hack

        paramsStack = nameStack[nameStack.index('(') + 1: ]
        if paramsStack: paramsStack = paramsStack[ : paramsStack.index(')') ]	# fixed april 7th
        #Remove things from the stack till we hit the last paren, this helps handle abstract and normal methods
        #if paramsStack:	# harts hacks - this is a bug caused by change to line 88?
        #    while paramsStack[-1]  != ")": paramsStack.pop()
        #    paramsStack.pop()

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
        self._params_helper( params )


class _CppVariable(dict):
	def _name_stack_helper( self, stack ):
		stack = list(stack)
		if '=' not in stack:		# TODO refactor me
			# check for array[n] and deal with funny array syntax: "int myvar:99"
			array = []
			while stack and stack[-1].isdigit(): array.append( stack.pop() )
			if array: array.reverse(); self['array'] = int(''.join(array))
			if stack and stack[-1].endswith(':'): stack[-1] = stack[-1][:-1]	# fixed June 23, BulletPhysics

		while stack and not stack[-1]: stack.pop()			# can be empty
		return stack

	def init(self):
		#assert self['name']	# allow unnamed variables, methods like this: "void func(void);"
		a = []
		self['aliases'] = []; self['parent'] = None; self['typedef'] = None
		for key in 'constant reference pointer static typedefs class fundamental unresolved'.split():
			self[ key ] = 0
		for b in self['type'].split():
			if b == '__const__': b = 'const'
			a.append( b )
		self['type'] = ' '.join( a )


class CppVariable( _CppVariable ):
    """Takes a name stack and turns it into a method
    
    Contains the following Keys:
    self['type'] - Type for the variable (ex. "const string &")
    self['name'] - Name of the variable (ex. "numItems")
    self['namespace'] - Namespace containing the enum
    self['desc'] - Description of the variable if part of a method (optional)
    self['doxygen'] - Doxygen comments associated with the method if they exist
    self['defaltValue'] - Default value of the variable, this key will only
        exist if there is a default value
    """
    Vars = []
    def __init__(self, nameStack,  **kwargs):
        nameStack = self._name_stack_helper( nameStack )
        if (debug): print( "Variable: ",  nameStack )
        if (len(nameStack) < 2): return

        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""
        if ("=" in nameStack):
            self["type"] = " ".join(nameStack[:nameStack.index("=") - 1])
            self["name"] = nameStack[nameStack.index("=") - 1]
            self["defaltValue"] = " ".join(nameStack[nameStack.index("=") + 1:])	# deprecate camelCase in dicts
            self['default'] = " ".join(nameStack[nameStack.index("=") + 1:])

        elif nameStack[-1] == '*':		# rare case - function param is an unnamed pointer: "void somemethod( SomeObject* )"
            self['type'] = ' '.join(nameStack)
            self['name'] = ''

        else:	# common case
            self["type"] = " ".join(nameStack[:-1])
            self["name"] = nameStack[-1]

        self["type"] = self["type"].replace(" :",":")
        self["type"] = self["type"].replace(": ",":")
        self["type"] = self["type"].replace(" <","<")
        self["type"] = self["type"].replace(" >",">")
        #Optional doxygen description
        try:
            self["desc"] = kwargs["doxyVarDesc"][self["name"]]
        except: pass

        self.init()
        CppVariable.Vars.append( self )		# save and resolve later

class _CppEnum(dict):
	def resolve_enum_values( self, values ):
		t = int; i = 0
		names = [ v['name'] for v in values ]
		for v in values:
			if 'value' in v:
				a = v['value'].strip()
				if a.isdigit(): i = a = int( a )
				elif a in names:
					for other in values:
						if other['name'] == a:
							v['value'] = other['value']
							break

				elif '"' in a or "'" in a: t = str	# only if there are quotes it this a string enum

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
            self['type'] = self.resolve_enum_values( valueList )	# returns int for standard enum
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
		if a not in 'size_t struct union unsigned signed bool char short int float double long void *': return False
	return True

def is_method_namestack(stack):
	r = False
	if '(' not in stack: r = False
	#elif '=' in stack and stack.index('=') < stack.index('(') and stack[stack.index('=')-1] != 'operator': r = False	#disabled July6th - allow all operators
	elif 'operator' in stack: r = True	# allow all operators
	elif '{' in stack and stack.index('{') < stack.index('('): r = False	# struct that looks like a method/class
	elif '(' in stack and ')' in stack:
		if '{' in stack and '}' in stack: r = True
		elif stack[-1] == ';': r = True
		elif '{' in stack: r = True	# ideally we catch both braces... TODO
	else: r = False
	return r


class CppStruct(dict):
	Structs = []
	def __init__(self, nameStack):
		if len(nameStack) >= 2: self['type'] = nameStack[1]
		else: self['type'] = None
		self['fields'] = []
		self.Structs.append( self )

C99_NONSTANDARD = {
	'int8' : 'signed char',
	'int16' : 'short int',
	'int32' : 'int',
	'int64' : 'int64_t',		# this can be: long int (64bit), or long long int (32bit)
	'uint' : 'unsigned int',
	'uint8' : 'unsigned char',
	'uint16' : 'unsigned short int',
	'uint32' : 'unsigned int',
	'uint64' : 'uint64_t',	# depends on host bits
}


def standardize_fundamental( s ):
	if s in C99_NONSTANDARD: return C99_NONSTANDARD[ s ]
	else: return s


class Resolver(object):
	C_FUNDAMENTAL = 'size_t unsigned signed bool char wchar short int float double long void'.split()
	C_FUNDAMENTAL += 'struct union enum'.split()


	SubTypedefs = {}		# TODO deprecate?
	NAMESPACES = []
	CLASSES = {}
	STRUCTS = {}

	def initextra(self):
		self.typedefs = {}
		self.typedefs_order = []
		self.classes_order = []
		self.structs = Resolver.STRUCTS
		self.structs_order = []
		self.namespaces = Resolver.NAMESPACES		# save all namespaces
		self.curStruct = None
		self.stack = []	# full name stack, good idea to keep both stacks? (simple stack and full stack)
		self._classes_brace_level = {}	# class name : level
		self._structs_brace_level = {}		# struct type : level
		self._method_body = None
		self._forward_decls = []
		self._template_typenames = []	# template<typename XXX>

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
		if 'long' in a and 'double' in a: b = 'longdouble'	# there is no ctypes.c_ulongdouble (this is a 64bit float?)
		elif a.count('long') == 2 and 'int' in a: b = '%sint64' %u
		elif a.count('long') == 2: b = '%slonglong' %u
		elif 'long' in a: b = '%slong' %u
		elif 'double' in a: b = 'double'	# no udouble in ctypes
		elif 'short' in a: b = '%sshort' %u
		elif 'char' in a: b = '%schar' %u
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

		elif string in 'struct union'.split(): b = 'void_p'	# what should be done here? don't trust struct, it could be a class, no need to expose via ctypes
		else: b = 'void_p'

		if not pointers: return 'ctypes.c_%s' %b
		else:
			x = ''
			for i in range(pointers): x += 'ctypes.POINTER('
			x += 'ctypes.c_%s' %b
			x += ')' * pointers
			return x

	def resolve_type( self, string, result ):	# recursive
		'''
		keeps track of useful things like: how many pointers, number of typedefs, is fundamental or a class, etc...
		'''
		## be careful with templates, what is inside <something*> can be a pointer but the overall type is not a pointer
		## these come before a template
		s = string.split('<')[0]
		result[ 'constant' ] += s.split().count('const')
		result[ 'static' ] += s.split().count('static')
		result[ 'mutable' ] = 'mutable' in s.split()

		## these come after a template
		s = string.split('>')[-1]
		result[ 'pointer' ] += s.count('*')
		result[ 'reference' ] += s.count('&')


		x = string; alias = False
		for a in '* & const static mutable'.split(): x = x.replace(a,'')
		for y in x.split():
			if y not in self.C_FUNDAMENTAL: alias = y; break

		#if alias == 'class':
		#	result['class'] = result['name']	# forward decl of class
		#	result['forward_decl'] = True
		if alias == '__extension__': result['fundamental_extension'] = True
		elif alias:
			result['aliases'].append( alias )
			if alias in C99_NONSTANDARD:
				result['type'] = C99_NONSTANDARD[ alias ]
				result['typedef'] = alias
				result['typedefs'] += 1
			elif alias in self.typedefs:
				result['typedefs'] += 1
				result['typedef'] = alias
				self.resolve_type( self.typedefs[alias], result )
			elif alias in self.classes:
				klass = self.classes[alias]; result['fundamental'] = False
				result['class'] = klass
				result['unresolved'] = False
			else: result['unresolved'] = True
		else:
			result['fundamental'] = True
			result['unresolved'] = False


	def finalize_vars(self):
		for s in CppStruct.Structs:	# vars within structs can be ignored if they do not resolve
			for var in s['fields']: var['parent'] = s['type']
		#for c in self.classes.values():
		#	for var in c.get_all_properties(): var['parent'] = c['name']

		## RESOLVE ##
		for var in CppVariable.Vars:
			self.resolve_type( var['type'], var )
			#if 'method' in var and var['method']['name'] ==  '_notifyCurrentCamera': print(var); assert 0

		# then find concrete type and best guess ctypes type #
		for var in CppVariable.Vars:	
			if not var['aliases']:	#var['fundamental']:
				var['ctypes_type'] = self.guess_ctypes_type( var['type'] )
			else:
				var['unresolved'] = False	# below may test to True
				if var['class']:
					var['ctypes_type'] = 'ctypes.c_void_p'
				else:
					assert var['aliases']
					tag = var['aliases'][0]

					klass = None
					nestedEnum = None
					nestedStruct = None
					nestedTypedef = None
					if 'method' in var:
						klass = var['method']['parent']
						if tag in var['method']['parent']._public_enums:
							nestedEnum = var['method']['parent']._public_enums[ tag ]
						elif tag in var['method']['parent']._public_structs:
							nestedStruct = var['method']['parent']._public_structs[ tag ]
						elif tag in var['method']['parent']._public_typedefs:
							nestedTypedef = var['method']['parent']._public_typedefs[ tag ]


					if '<' in tag:	# should also contain '>'
						var['template'] = tag		# do not resolve templates
						var['ctypes_type'] = 'ctypes.c_void_p'
						var['unresolved'] = True

					elif nestedEnum:
						enum = nestedEnum
						if enum['type'] is int:
							var['ctypes_type'] = 'ctypes.c_int'
							var['raw_type'] = 'int'

						elif enum['type'] is str:
							var['ctypes_type'] = 'ctypes.c_char_p'
							var['raw_type'] = 'char*'

						var['enum'] = var['method']['path'] + '::' + enum['name']
						var['fundamental'] = True

					elif nestedStruct:
						var['ctypes_type'] = 'ctypes.c_void_p'
						var['raw_type'] = var['method']['path'] + '::' + nestedStruct['type']
						var['fundamental'] = False

					elif nestedTypedef:
						var['fundamental'] = is_fundamental( nestedTypedef )
						if not var['fundamental']:
							var['raw_type'] = var['method']['path'] + '::' + tag

					else:
						_tag = tag
						if '::' in tag and tag.split('::')[0] in self.namespaces: tag = tag.split('::')[-1]
						con = self.concrete_typedef( _tag )
						if con:
							var['concrete_type'] = con
							var['ctypes_type'] = self.guess_ctypes_type( var['concrete_type'] )

						elif tag in self.structs:
							print( 'STRUCT', var )
							var['struct'] = tag
							var['ctypes_type'] = 'ctypes.c_void_p'
							var['raw_type'] = self.structs[tag]['namespace'] + '::' + tag

						elif tag in self._forward_decls:
							var['forward_declared'] = tag
							var['ctypes_type'] = 'ctypes.c_void_p'

						elif tag in self.global_enums:
							enum = self.global_enums[ tag ]
							if enum['type'] is int:
								var['ctypes_type'] = 'ctypes.c_int'
								var['raw_type'] = 'int'
							elif enum['type'] is str:
								var['ctypes_type'] = 'ctypes.c_char_p'
								var['raw_type'] = 'char*'
							var['enum'] = enum['namespace'] + enum['name']
							var['fundamental'] = True


						elif var['parent']:
							print( 'WARN unresolved', _tag)
							var['ctypes_type'] = 'ctypes.c_void_p'
							var['unresolved'] = True


						elif tag.count('::')==1:
							print( 'trying to find nested something in', tag )
							a = tag.split('::')[0]
							b = tag.split('::')[-1]
							if a in self.classes:	# a::b is most likely something nested in a class
								klass = self.classes[ a ]
								if b in klass._public_enums:
									print( '...found nested enum', b )
									enum = klass._public_enums[ b ]
									if enum['type'] is int:
										var['ctypes_type'] = 'ctypes.c_int'
										var['raw_type'] = 'int'
									elif enum['type'] is str:
										var['ctypes_type'] = 'ctypes.c_char_p'
										var['raw_type'] = 'char*'
									if 'method' in var: var['enum'] = var['method']['path'] + '::' + enum['name']
									else:	# class property
										var['unresolved'] = True
									var['fundamental'] = True

								else: var['unresolved'] = True	# TODO klass._public_xxx

							elif a in self.namespaces:	# a::b can also be a nested namespace
								if b in self.global_enums:
									enum = self.global_enums[ b ]
									print(enum)
								print(var)
								assert 0

							elif b in self.global_enums:		# falling back, this is a big ugly
								enum = self.global_enums[ b ]
								assert a in enum['namespace'].split('::')
								if enum['type'] is int:
									var['ctypes_type'] = 'ctypes.c_int'
									var['raw_type'] = 'int'
								elif enum['type'] is str:
									var['ctypes_type'] = 'ctypes.c_char_p'
									var['raw_type'] = 'char*'
								var['fundamental'] = True

							else:	# boost::gets::crazy
								print('NAMESPACES', self.namespaces)
								print( a, b )
								print( '---- boost gets crazy ----' )
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

						elif tag in self.SubTypedefs:	# TODO remove SubTypedefs
							if 'property_of_class' in var or 'property_of_struct' in var:
								print( 'class:', self.SubTypedefs[ tag ], 'tag:', tag )
								var['typedef'] = self.SubTypedefs[ tag ]	# class name
								var['ctypes_type'] = 'ctypes.c_void_p'
							else:
								print( "WARN-this should almost never happen!" )
								print( var ); print('-'*80)
								var['unresolved'] = True

						elif tag in self._template_typenames:
							var['typename'] = tag
							var['ctypes_type'] = 'ctypes.c_void_p'
							var['unresolved'] = True	# TODO, how to deal with templates?

						elif tag.startswith('_'):	# assume starting with underscore is not important for wrapping
							print( 'WARN unresolved', _tag)
							var['ctypes_type'] = 'ctypes.c_void_p'
							var['unresolved'] = True

						else:
							print( 'WARN: unknown type', var )
							assert 'property_of_class' in var or 'property_of_struct'	# only allow this case
							var['unresolved'] = True


					## if not resolved and is a method param, not going to wrap these methods  ##
					if var['unresolved'] and 'method' in var: var['method']['unresolved_parameters'] = True


		# create stripped raw_type #
		p = '* & const static'.split()
		for var in CppVariable.Vars:
			if 'raw_type' in var: continue

			raw = []
			for x in var['type'].split():
				if x not in p: raw.append( x )
			var['raw_type'] = ' '.join( raw )

			#if 'AutoConstantEntry' in var['raw_type']: print(var); assert 0
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
				else: print('-'*80); print(var); raise NotImplemented

			#if var['raw_type'] == 'Animation': print(var); assert 0

			## need full name space for classes in raw type ##
			#if var['class'] and '::' not in var['raw_type'] and var['class']['namespace']:



	def concrete_typedef( self, key ):
		if key not in self.typedefs:
			#print( 'FAILED typedef', key )
			return None
		while key in self.typedefs:
			prev = key
			key = self.typedefs[ key ]
			if '<' in key or '>' in key: return prev		# stop at template
			if key.startswith('std::'): return key		# stop at std lib
		return key


class _CppHeader( Resolver ):
	def finalize(self):
		self.finalize_vars()
		# finalize classes and method returns types
		for cls in self.classes.values():
			for meth in cls.get_all_methods():
				if meth['pure_virtual']: cls['abstract'] = True

				if not meth['returns_fundamental'] and meth['returns'] in C99_NONSTANDARD:
					meth['returns'] = C99_NONSTANDARD[meth['returns']]
					meth['returns_fundamental'] = True

				elif not meth['returns_fundamental']:	# describe the return type
					con = None
					if cls['namespace'] and '::' not in meth['returns']:
						con = self.concrete_typedef( cls['namespace'] + '::' + meth['returns'] )
					else: con = self.concrete_typedef( meth['returns'] )


					if con:
						meth['returns_concrete'] = con
						meth['returns_fundamental'] = is_fundamental( con )

					elif meth['returns'] in self.classes:
						print( 'meth returns class:', meth['returns'] )
						meth['returns_class'] = True

					elif meth['returns'] in self.SubTypedefs:
						meth['returns_class'] = True
						meth['returns_nested'] = self.SubTypedefs[ meth['returns'] ]

					elif meth['returns'] in cls._public_enums:
						enum = cls._public_enums[ meth['returns'] ]
						meth['returns_enum'] = enum['type']
						meth['returns_fundamental'] = True
						if enum['type'] == int: meth['returns'] = 'int'
						else: meth['returns'] = 'char*'

					elif meth['returns'] in self.global_enums:
						enum = self.global_enums[ meth['returns'] ]
						meth['returns_enum'] = enum['type']
						meth['returns_fundamental'] = True
						if enum['type'] == int: meth['returns'] = 'int'
						else: meth['returns'] = 'char*'

					elif meth['returns'].count('::')==1:
						print( meth )
						a,b = meth['returns'].split('::')
						if a in self.namespaces:
							if b in self.classes:
								klass = self.classes[ b ]
								meth['returns_class'] = a + '::' + b
							elif '<' in b and '>' in b:
								print( 'WARN-can not return template:', b )
								meth['returns_unknown'] = True
							elif b in self.global_enums:
								enum = self.global_enums[ b ]
								meth['returns_enum'] = enum['type']
								meth['returns_fundamental'] = True
								if enum['type'] == int: meth['returns'] = 'int'
								else: meth['returns'] = 'char*'

							else: print( a, b); print( meth); assert 0

						elif a in self.classes:
							klass = self.classes[ a ]
							if b in klass._public_enums:
								print( '...found nested enum', b )
								enum = klass._public_enums[ b ]
								meth['returns_enum'] = enum['type']
								meth['returns_fundamental'] = True
								if enum['type'] == int: meth['returns'] = 'int'
								else: meth['returns'] = 'char*'

							elif b in klass._public_forward_declares:
								meth['returns_class'] = True

							elif b in klass._public_typedefs:
								typedef = klass._public_typedefs[ b ]
								meth['returns_fundamental'] = is_fundamental( typedef )

							else: print( meth ); raise NotImplemented

					elif '::' in meth['returns']:
						print('TODO namespace or extra nested return:', meth)
						meth['returns_unknown'] = True
					else:
						print( 'WARN: UNKNOWN RETURN', meth['name'], meth['returns'])
						meth['returns_unknown'] = True

		for cls in self.classes.values():
			methnames = cls.get_all_method_names()
			pvm = cls.get_all_pure_virtual_methods()

			for d in cls['inherits']:
				c = d['class']
				a = d['access']	# do not depend on this to be 'public'
				print( 'PARENT CLASS:', c )
				if c not in self.classes: print('WARN: parent class not found')
				if c in self.classes and self.classes[c]['abstract']:
					p = self.classes[ c ]
					for meth in p.get_all_methods():	#p["methods"]["public"]:
						print( '\t\tmeth', meth['name'], 'pure virtual', meth['pure_virtual'] )
						if meth['pure_virtual'] and meth['name'] not in methnames: cls['abstract'] = True; break





	def evaluate_struct_stack(self):
		"""Create a Struct out of the name stack (but not its parts)"""
		#print( 'eval struct stack', self.nameStack )
		#if self.braceDepth != len(self.nameSpaces): return
		struct = CppStruct(self.nameStack)
		struct["namespace"] = self.cur_namespace()
		self.structs[ struct['type'] ] = struct
		self.structs_order.append( struct )
		if self.curClass:
			struct['parent'] = self.curClass
			klass = self.classes[ self.curClass ]
			klass['structs'][self.curAccessSpecifier].append( struct )
			if self.curAccessSpecifier == 'public': klass._public_structs[ struct['type'] ] = struct
		self.curStruct = struct
		self._structs_brace_level[ struct['type'] ] = self.braceDepth

	## python style ##
	PYTHON_OPERATOR_MAP = {
		'()' : '__call__',
		'[]' : '__getitem__',
		'<'	:	'__lt__',
		'<='	:	'__le__',
		'=='	:	'__eq__',
		'!='	:	'__ne__',
		'>'	:	'__gt__',
		'>='	:	'__ge__',
		'+'	:	'__add__',
		'-'	:	'__sub__',
		'*'	:	'__mul__',
		'%'	:	'__divmod__',
		'**'	:	'__pow__',
		'>>'	:	'__lshift__',
		'<<'	:	'__rshift__',
		'&'	:	'__and__',
		'^'	:	'__xor__',
		'|'	:	'__or__',
		'+='	:	'__iadd__',
		'-='	:	'__isub__',
		'*='	:	'__imult__',
		'/='	:	'__idiv__',
		'%='	:	'__imod__',
		'**='	:	'__ipow__',
		'<<='	:	'__ilshift__',
		'>>='	:	'__irshift__',
		'&='	:	'__iand__',
		'^='	:	'__ixor__',
		'|='	:	'__ior__',
		#__neg__ __pos__ __abs__; what are these in c++?
		'~'	:	'__invert__',
		'.'	:	'__getattr__',
	}
	OPERATOR_MAP = {	# do not use double under-scores so no conflicts with python #
		'='	:	'_assignment_',
		'->'	:	'_select_member_',
		'++'	:	'_increment_',
		'--'	:	'_deincrement_',
		'new'	:	'_new_',
		'delete' :	'_delete_',
	}
	OPERATOR_MAP.update( PYTHON_OPERATOR_MAP )

	def parse_method_type( self, stack ):
		#print( 'meth type info', stack )
		if stack[0] in ':;': stack = stack[1:]
		info = { 
			'debug': ' '.join(stack), 
			'class':None, 
			'namespace':self.cur_namespace(add_double_colon=True),
		}

		for tag in 'defined pure_virtual operator constructor destructor extern template virtual static explicit inline friend returns returns_pointer returns_fundamental returns_class'.split(): info[tag]=False
		header = stack[ : stack.index('(') ]
		header = ' '.join( header )
		header = header.replace(' : : ', '::' )
		header = header.replace(' < ', '<' )
		header = header.replace(' > ', '> ' )
		header = header.strip()

		if '{' in stack:
			info['defined'] = True
			self._method_body = self.braceDepth
			print( 'NEW METHOD WITH BODY', self.braceDepth )
		elif stack[-1] == ';':
			info['defined'] = False
			self._method_body = None	# not a great idea to be clearing here
		else: assert 0

		if len(stack) > 3 and stack[-1] == ';' and stack[-2] == '0' and stack[-3] == '=':
			info['pure_virtual'] = True

		r = header.split()
		name = None
		if 'operator' in stack:	# rare case op overload defined outside of class
			op = stack[ stack.index('operator')+1 : stack.index('(') ]
			op = ''.join(op)
			if not op:
				print( 'TODO - parse [] and () operators' )
				return None
			else:
				info['operator'] = op
				if op in self.OPERATOR_MAP:
					name = '__operator__' + self.OPERATOR_MAP[ op ]
					a = stack[ : stack.index('operator') ]
				else:
					print('ERROR - not a C++ operator', op)
					return None

		elif r:		# June 23 2011
			name = r[-1]
			a = r[ : -1 ]	# strip name

		if name is None: return None
		#if name.startswith('~'): name = name[1:]

		while a and a[0] == '}':	# strip - can have multiple } }
			a = a[1:]	# july3rd


		if '::' in name:
			#klass,name = name.split('::')	# methods can be defined outside of class
			klass = name[ : name.rindex('::') ]
			name = name.split('::')[-1]
			info['class'] = klass
		#	info['name'] = name
		#else: info['name'] = name

		if name.startswith('~'):
			info['destructor'] = True
			name = name[1:]
		elif not a:
			info['constructor'] = True

		info['name'] = name

		for tag in 'extern virtual static explicit inline friend'.split():
			if tag in a: info[ tag ] = True; a.remove( tag )	# inplace
		if 'template' in a:
			a.remove('template')
			b = ' '.join( a )
			if '>' in b:
				info['template'] = b[ : b.index('>')+1 ]
				info['returns'] = b[ b.index('>')+1 : ]	# find return type, could be incorrect... TODO
				if '<typename' in info['template'].split():
					typname = info['template'].split()[-1]
					typname = typname[ : -1 ]	# strip '>'
					if typname not in self._template_typenames: self._template_typenames.append( typname )
			else: info['returns'] = ' '.join( a )
		else: info['returns'] = ' '.join( a )
		info['returns'] = info['returns'].replace(' <', '<').strip()

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

		if self.curStruct:
			print( 'WARN - struct contains methods - skipping' )
			print( self.stack )
			assert 0

		info = self.parse_method_type( self.stack )
		if info:
			if info[ 'class' ] and info['class'] in self.classes:	 # case where methods are defined outside of class
				newMethod = CppMethod(self.nameStack, info['name'], info)
				klass = self.classes[ info['class'] ]
				klass[ 'methods' ][ 'public' ].append( newMethod )
				newMethod['parent'] = klass
				if klass['namespace']: newMethod['path'] = klass['namespace'] + '::' + klass['name']
				else: newMethod['path'] = klass['name']

			elif self.curClass:	# normal case
				newMethod = CppMethod(self.nameStack, self.curClass, info)
				klass = self.classes[self.curClass]
				klass['methods'][self.curAccessSpecifier].append(newMethod)
				newMethod['parent'] = klass
				if klass['namespace']: newMethod['path'] = klass['namespace'] + '::' + klass['name']
				else: newMethod['path'] = klass['name']

		else:
			print( 'free function?', self.nameStack )

		self.stack = []

	def _parse_typedef( self, stack, namespace='' ):
		if not stack or 'typedef' not in stack: return
		stack = list( stack )	# copy just to be safe
		if stack[-1] == ';': stack.pop()

		while stack and stack[-1].isdigit(): stack.pop()	# throw away array size for now

		idx = stack.index('typedef')
		name = namespace + stack[-1]
		s = ''
		for a in stack[idx+1:-1]:
			if a == '{': break
			if not s or s[-1] in ':<>' or a in ':<>': s += a	# keep compact
			else: s += ' ' + a	# spacing

		r = {'name':name, 'raw':s, 'type':s}
		if not is_fundamental(s):
			if 'struct' in s.split(): pass		# TODO is this right? "struct ns::something"
			elif '::' not in s: s = namespace + s 		# only add the current name space if no namespace given
			#elif '::' in s:
			#	ns = s.split('::')[0]
			#	if ns not in self.namespaces:
			r['type'] = s
		if s: return r


	def evaluate_typedef(self):
		ns = self.cur_namespace(add_double_colon=True)
		res = self._parse_typedef( self.stack, ns )
		if res:
			name = res['name']
			self.typedefs[ name ] = res['type']
			if name not in self.typedefs_order: self.typedefs_order.append( name )


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
			newVar = CppVariable(self.nameStack)
			newVar['namespace'] = self.current_namespace()
			if self.curStruct:
				self.curStruct[ 'fields' ].append( newVar )
				newVar['property_of_struct'] = self.curStruct
			elif self.curClass:
				klass = self.classes[self.curClass]
				klass["properties"][self.curAccessSpecifier].append(newVar)
				newVar['property_of_class'] = klass['name']

		self.stack = []		# CLEAR STACK

	def evaluate_class_stack(self):
		"""Create a Class out of the name stack (but not its parts)"""
		#dont support sub classes today
		#print( 'eval class stack', self.nameStack )
		parent = self.curClass
		if self.braceDepth > len( self.nameSpaces) and parent:
			print( 'HIT NESTED SUBCLASS' )
		elif self.braceDepth != len(self.nameSpaces):
			print( 'ERROR: WRONG BRACE DEPTH' )
			return

		self.curAccessSpecifier = 'private'		# private is default
		newClass = CppClass(self.nameStack)
		print( 'NEW CLASS', newClass['name'] )
		self.classes_order.append( newClass )	# good idea to save ordering
		self.stack = []		# fixes if class declared with ';' in closing brace
		if parent:
			newClass["namespace"] = self.classes[ parent ]['namespace'] + '::' + parent
			newClass['parent'] = parent
			self.classes[ parent ]['nested_classes'].append( newClass )
			## supports nested classes with the same name ##
			self.curClass = key = parent+'::'+newClass['name']
			self._classes_brace_level[ key ] = self.braceDepth

		elif newClass['parent']:		# nested class defined outside of parent.  A::B {...}
			parent = newClass['parent']
			newClass["namespace"] = self.classes[ parent ]['namespace'] + '::' + parent
			self.classes[ parent ]['nested_classes'].append( newClass )
			## supports nested classes with the same name ##
			self.curClass = key = parent+'::'+newClass['name']
			self._classes_brace_level[ key ] = self.braceDepth

		else:
			newClass["namespace"] = self.cur_namespace()
			key = newClass['name']
			self.curClass = newClass["name"]
			self._classes_brace_level[ newClass['name'] ] = self.braceDepth

		if key in self.classes:
			print( 'ERROR name collision:', key )
			self.classes[key].show()
			print('-'*80)
			newClass.show()

		assert key not in self.classes	# namespace collision
		self.classes[ key ] = newClass

	def evalute_forward_decl(self):
		print( 'FORWARD DECL', self.nameStack )
		assert self.nameStack[0] == 'class'
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

    def __init__(self, headerFileName, argType="file"):
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

        # nested classes have parent::nested, but no extra namespace,
        # this keeps the API compatible, TODO proper namespace for everything. 
        Resolver.CLASSES = {}
        self.classes = Resolver.CLASSES

        self.enums = []
        self.global_enums = {}
        self.nameStack = []
        self.nameSpaces = []
        self.curAccessSpecifier = 'private'	# private is default
        self.initextra()	# harts hack
    
        if (len(self.headerFileName)):
            headerFileStr = "\n".join(open(self.headerFileName).readlines())
        self.braceDepth = 0
        lex.input(headerFileStr)
        curLine = 0
        curChar = 0
        if 1:	#try:
            while True:
                tok = lex.token()
                if not tok: break
                if tok.type == 'NAME' and tok.value in self.IGNORE_NAMES: continue
                if tok.type not in ('PRECOMP_MACRO', 'PRECOMP_MACRO_CONT'): self.stack.append( tok.value )
                curLine = tok.lineno
                curChar = tok.lexpos
                if (tok.type == 'OPEN_BRACE'):
                    if len(self.nameStack) >= 2 and is_namespace(self.nameStack):	# namespace {} with no name used in boost, this sets default?
                        self.nameSpaces.append(self.nameStack[1])
                        ns = self.cur_namespace(); self.stack = []
                        if ns not in self.namespaces: self.namespaces.append( ns )
                    if len(self.nameStack) and not is_enum_namestack(self.nameStack):
                        self.evaluate_stack()
                    else:
                        self.nameStack.append(tok.value)
                    if self.stack and self.stack[0] == 'class': self.stack = []
                    self.braceDepth += 1
                elif (tok.type == 'CLOSE_BRACE'):
                    if self.braceDepth == 0:
                        continue
                    if (self.braceDepth == len(self.nameSpaces)):
                        tmp = self.nameSpaces.pop()
                        self.stack = []	# clear stack when namespace ends?
                    if len(self.nameStack) and is_enum_namestack(self.nameStack):
                        self.nameStack.append(tok.value)
                    elif self.braceDepth < 10:
                        self.evaluate_stack()
                    else:
                        self.nameStack = []
                    self.braceDepth -= 1
                    #self.stack = []; print 'BRACE DEPTH', self.braceDepth, 'NS', len(self.nameSpaces)	# June29 2011
                    if self.curClass and debug: print( 'CURBD', self._classes_brace_level[ self.curClass ] )

                    if (self.braceDepth == 0) or (self.curClass and self._classes_brace_level[self.curClass]==self.braceDepth):
                        print( 'END OF CLASS DEF' )
                        if self.curClass and self.classes[ self.curClass ]['parent']: self.curClass = self.classes[ self.curClass ]['parent']
                        else: self.curClass = ""; #self.curStruct = None
                        self.stack = []

                    #if self.curStruct: self.curStruct = None
                    if self.braceDepth == 0 or (self.curStruct and self._structs_brace_level[self.curStruct['type']]==self.braceDepth):
                        print( 'END OF STRUCT DEF' )
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
                    if (tok.value == 'class'):
                        self.nameStack.append(tok.value)
                    elif tok.value in supportedAccessSpecifier:
                        if self.braceDepth == len(self.nameSpaces) + 1: self.curAccessSpecifier = tok.value; 
                        self.stack = []
                    else:
                        self.nameStack.append(tok.value)
                elif (tok.type == 'COLON'):
                    #Dont want colon to be first in stack
                    if len(self.nameStack) == 0:
                        continue
                    self.nameStack.append(tok.value)

                elif (tok.type == 'SEMI_COLON'):
                    if (self.braceDepth < 10): self.evaluate_stack( tok.type )
                    if not self.stack: continue
                    if self.stack[0]=='typedef' and ( '{' not in self.stack or '}' in self.stack ): self.stack = []; print( "REAL CLEAR")
                    elif self.stack[0] != 'typedef': self.stack = []; print('CLEAR STACK')

        #except:
        #    raise CppParseError("Not able to parse %s on line %d evaluating \"%s\"\nError around: %s"
        #                        % (self.headerFileName, tok.lineno, tok.value, " ".join(self.nameStack)))

        self.finalize()

    def evaluate_stack(self, token=None):
        """Evaluates the current name stack"""
        global doxygenCommentCache
        if (debug): print( "Evaluating stack %s\nBraceDepth: %s" %(self.nameStack,self.braceDepth))
        if (len(self.curClass)):
            if (debug): print( "%s (%s) "%(self.curClass, self.curAccessSpecifier))

        #if 'typedef' in self.nameStack: self.evaluate_typedef()		# allows nested typedefs, probably a bad idea
        if not self.curClass and 'typedef' in self.nameStack:
            print('STACK', self.stack)
            if token == 'SEMI_COLON' and ('{' not in self.stack or '}' in self.stack): self.evaluate_typedef()
            else: return

        elif (len(self.nameStack) == 0):
            if (debug): print( "line ",lineno() )
            if (debug): print( "(Empty Stack)" )
            return
        elif (self.nameStack[0] == "namespace"):
            #Taken care of outside of here
            pass
        elif len(self.nameStack) >= 2 and self.nameStack[0] == 'using' and self.nameStack[1] == 'namespace': pass	# TODO

        elif is_enum_namestack(self.nameStack):
            if (debug): print( "line ",lineno() )
            self.evaluate_enum_stack()

        elif self._method_body and self.braceDepth > self._method_body: print( 'INSIDE METHOD DEF' )
        elif is_method_namestack(self.stack) and not self.curStruct and '(' in self.nameStack:	# updated by hart
            if (debug): print( "line ",lineno() )
            self.evaluate_method_stack()
        elif '(' not in self.nameStack and ')' not in self.nameStack and self.stack[-1] == ';':
            if (debug): print( "line ",lineno() )
            if self.nameStack[0]=='class': self.evalute_forward_decl()
            elif len(self.nameStack) >= 2 and (self.nameStack[0]=='friend' and self.nameStack[1]=='class'): pass
            else: self.evaluate_property_stack()	# catches class props and structs in a namespace

        elif (self.nameStack[0] == "class"):
            if (debug): print( "line ",lineno() )
            self.evaluate_class_stack()
        elif (self.nameStack[0] == "struct"):
            if (debug): print( "line ",lineno() )
            ##this causes a bug when structs are nested in protected or private##self.curAccessSpecifier = "public"
            self.evaluate_struct_stack()	# hart's hack - do structs properly


        elif not self.curClass:
            if (debug): print( "line ",lineno() )
            if is_enum_namestack(self.nameStack): self.evaluate_enum_stack()
            elif self.curStruct and self.stack[-1] == ';': self.evaluate_property_stack()	# this catches fields of global structs
            self.nameStack = []
            doxygenCommentCache = ""
            return
        elif (self.braceDepth < 1):
            if (debug): print( "line ",lineno() )
            #Ignore global stuff for now
            if (debug): print( "Global stuff: ",  self.nameStack )
            self.nameStack = []
            doxygenCommentCache = ""
            return
        elif (self.braceDepth > len(self.nameSpaces) + 1):
            if (debug): print( "line ",lineno() )
            self.nameStack = []
            doxygenCommentCache = ""
            return

        self.nameStack = []		# its a little confusing to have some if/else above return and others not, and then clearning the nameStack down here
        doxygenCommentCache = ""
    

    def evaluate_enum_stack(self):
        """Create an Enum out of the name stack"""
        newEnum = CppEnum(self.nameStack)
        if len(newEnum.keys()):
            if len(self.curClass):
                newEnum["namespace"] = self.cur_namespace(True)
                klass = self.classes[self.curClass]
                klass["enums"][self.curAccessSpecifier].append(newEnum)
                if self.curAccessSpecifier == 'public' and 'name' in newEnum: klass._public_enums[ newEnum['name'] ] = newEnum
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



