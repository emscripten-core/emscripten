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
"""Parse C++ header files and generate a data structure
representing the class
"""

import ply.lex as lex
import os
import sys
import re

import inspect

def lineno():
    """Returns the current line number in our program."""
    return inspect.currentframe().f_back.f_lineno


__version__ = "1.9"
version = "1.9"
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

t_ignore = " \t\r[].|!?%@"
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
    print "Lex error: ", v

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
    def __init__(self, nameStack):
        if (debug): print "Class:   ",  nameStack
        if (len(nameStack) < 2):
            print "Error detecting class"
            return
        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""
        self["name"] = nameStack[1]
        inheritList = []
        if ":" in nameStack:
            nameStack = nameStack[nameStack.index(":") + 1:]
            while len(nameStack):
                tmpStack = []
                tmpInheritClass = {"access":"private"}
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
                    print "Warning: Cant figure out class inheriting %s\n"%(" ".join(tmpStack))
                    continue
                inheritList.append(tmpInheritClass)
        methodAccessSpecificList = {}
        propertyAccessSpecificList = {}
        enumAccessSpecificList = {}
        
        for accessSpecifier in supportedAccessSpecifier:
            methodAccessSpecificList[accessSpecifier] = []
            propertyAccessSpecificList[accessSpecifier] = []
            enumAccessSpecificList[accessSpecifier] = []
        self['inherits'] = inheritList
        self['methods'] = methodAccessSpecificList
        self['properties'] = propertyAccessSpecificList
        self['enums'] = enumAccessSpecificList
        self['namespace'] = ""

    def __repr__(self):
        """Convert class to a string"""
        namespace_prefix = ""
        if self["namespace"]: namespace_prefix = self["namespace"] + "::"
        rtn = "class %s\n"%(namespace_prefix + self["name"])
        try:
            print self["doxygen"], 
        except: pass
        if "inherits" in self.keys():
            rtn += "Inherits: "
            for inheritClass in self["inherits"]:
                rtn += "%s %s, "%(inheritClass["access"], inheritClass["class"])
            rtn += "\n"
        rtn += "{\n"
        for accessSpecifier in supportedAccessSpecifier:
            rtn += "%s\n"%(accessSpecifier)
            #Enums
            if (len(self["enums"][accessSpecifier])):
                rtn += "    // Enums\n"
            for enum in self["enums"][accessSpecifier]:
                rtn += "    %s\n"%(repr(enum))
            #Properties
            if (len(self["properties"][accessSpecifier])):
                rtn += "    // Properties\n"
            for property in self["properties"][accessSpecifier]:
                rtn += "    %s\n"%(repr(property))
            #Methods
            if (len(self["methods"][accessSpecifier])):
                rtn += "    // Method\n"
            for method in self["methods"][accessSpecifier]:
                rtn += "    %s\n"%(repr(method))
        rtn += "}\n"
        return rtn

class CppMethod(dict):
    """Takes a name stack and turns it into a method
    
    Contains the following Keys:
    self['rtnType'] - Return type of the method (ex. "int")
    self['name'] - Name of the method (ex. "getSize")
    self['doxygen'] - Doxygen comments associated with the method if they exist
    self['parameters'] - List of CppVariables
    """
    def __init__(self, nameStack, curClass):
        if (debug): print "Method:   ",  nameStack
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
        paramsStack = nameStack[nameStack.index('(') + 1: ]
        #Remove things from the stack till we hit the last paren, this helps handle abstract and normal methods
        while (paramsStack[-1]  != ")"):
            paramsStack.pop()
        paramsStack.pop()
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
                params.append(CppVariable(paramsStack[0:paramsStack.index(',')],  doxyVarDesc=doxyVarDesc))
                paramsStack = paramsStack[paramsStack.index(',') + 1:]
            else:
                param = CppVariable(paramsStack,  doxyVarDesc=doxyVarDesc)
                if len(param.keys()):
                    params.append(param)
                break
        self["parameters"] = params


class CppVariable(dict):
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
    def __init__(self, nameStack,  **kwargs):
        if (debug): print "Variable: ",  nameStack
        if (len(nameStack) < 2):
            return
        global doxygenCommentCache
        if len(doxygenCommentCache):
            self["doxygen"] = doxygenCommentCache
            doxygenCommentCache = ""
        if ("=" in nameStack):
            self["type"] = " ".join(nameStack[:nameStack.index("=") - 1])
            self["name"] = nameStack[nameStack.index("=") - 1]
            self["defaltValue"] = " ".join(nameStack[nameStack.index("=") + 1:])
        else:
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

class CppEnum(dict):
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
            if len(tmpStack) == 1:
                valueList.append({"name": tmpStack[0]})
            elif len(tmpStack) >= 3 and tmpStack[1] == "=":
                valueList.append({"name": tmpStack[0], "value": " ".join(tmpStack[2:])})
            elif len(tmpStack) == 2 and tmpStack[1] == "=":
                if (debug): print "Missed value for %s"%tmpStack[0]
                valueList.append({"name": tmpStack[0]})
        if len(valueList):
            self["values"] = valueList
        else:
            #An enum without any values is useless, dont bother existing
            return
        #Figure out if it has a name
        preBraceStack = nameStack[:nameStack.index("{")]
        postBraceStack = nameStack[nameStack.index("}") + 1:]
        if (len(preBraceStack) == 2 and "typedef" not in nameStack):
            self["name"] = preBraceStack[1]           
        elif len(postBraceStack) and "typedef" in nameStack:
                self["name"] = " ".join(postBraceStack)
        #See if there are instances of this
        if "typedef" not in nameStack and len(postBraceStack):
            self["instances"] = []
            for var in postBraceStack:
                if "," in var:
                    continue
                self["instances"].append(var)
        self["namespace"] = ""

class CppHeader:
    """Parsed C++ class header
    
    Variables produced:
    self.classes - Dictionary of classes found in a given header file where the
        key is the name of the class
    """
    def __init__(self, headerFileName, argType = "file"):
        if (argType == "file"):
            self.headerFileName = os.path.expandvars(headerFileName)
            self.mainClass = os.path.split(self.headerFileName)[1][:-2]
            headerFileStr = ""
#            if headerFileName[-2:] != ".h":
#                raise Exception("file must be a header file and end with .h")
        elif argType == "string":
            self.headerFileName = ""
            self.mainClass = "???"
            headerFileStr = headerFileName
        else:
            raise Exception("Arg type must be either file or string")
        self.curClass = ""
        self.classes = {}
        self.enums = []
        self.nameStack = []
        self.nameSpaces = []
        self.curAccessSpecifier = 'private'
    
        if (len(self.headerFileName)):
            headerFileStr = "\n".join(open(self.headerFileName).readlines())
        self.braceDepth = 0
        lex.input(headerFileStr)
        curLine = 0
        curChar = 0
        try:
            while True:
                tok = lex.token()
                # Example: LexToken(COLON,';',1,373)
                # where (tok.name, tok.value, ?, ?)
                if not tok:
                    break
                curLine = tok.lineno
                curChar = tok.lexpos
                if (tok.type == 'OPEN_BRACE'):
                    if len(self.nameStack) and is_namespace(self.nameStack):
                        self.nameSpaces.append(self.nameStack[1])
                    if len(self.nameStack) and not is_enum_namestack(self.nameStack):
                        self.evaluate_stack()
                    else:
                        self.nameStack.append(tok.value)
                    self.braceDepth += 1
                elif (tok.type == 'CLOSE_BRACE'):
                    if self.braceDepth == 0:
                        continue
                    if (self.braceDepth == len(self.nameSpaces)):
                        tmp = self.nameSpaces.pop()
                    if len(self.nameStack) and is_enum_namestack(self.nameStack):
                        self.nameStack.append(tok.value)
                    elif self.braceDepth < 10:
                        self.evaluate_stack()
                    else:
                        self.nameStack = []
                    self.braceDepth -= 1
                    if (self.braceDepth == 0):
                        self.curClass = ""
                
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
                    elif (tok.value in supportedAccessSpecifier and self.braceDepth == len(self.nameSpaces) + 1):
                        self.curAccessSpecifier = tok.value
                    else:
                        self.nameStack.append(tok.value)
                elif (tok.type == 'COLON'):
                    #Dont want colon to be first in stack
                    if len(self.nameStack) == 0:
                        continue
                    self.nameStack.append(tok.value)
                elif (tok.type == 'SEMI_COLON'):
                    if (self.braceDepth < 10):
                        self.evaluate_stack()
        except:
            raise CppParseError("Not able to parse %s on line %d evaluating \"%s\"\nError around: %s"
                                % (self.headerFileName, tok.lineno, tok.value, " ".join(self.nameStack)))
        
    def evaluate_stack(self):
        """Evaluates the current name stack"""
        global doxygenCommentCache
        if (debug): print "Evaluating stack %s at..."%self.nameStack
        if (len(self.curClass)):
            if (debug): print "%s (%s) "%(self.curClass, self.curAccessSpecifier), 
        if (len(self.nameStack) == 0):
            if (debug): print "line ",lineno()
            if (debug): print "(Empty Stack)"
            return
        elif (self.nameStack[0] == "namespace"):
            #Taken care of outside of here
            pass
        elif (self.nameStack[0] == "class"):
            if (debug): print "line ",lineno()
            self.evaluate_class_stack()
        elif (self.nameStack[0] == "struct"):
            if (debug): print "line ",lineno()
            self.curAccessSpecifier = "public"
            self.evaluate_class_stack()
        elif (len(self.curClass) == 0):
            if (debug): print "line ",lineno()
            if is_enum_namestack(self.nameStack):
                self.evaluate_enum_stack()
            self.nameStack = []
            doxygenCommentCache = ""
            return
        elif (self.braceDepth < 1):
            if (debug): print "line ",lineno()
            #Ignore global stuff for now
            if (debug): print "Global stuff: ",  self.nameStack
            self.nameStack = []
            doxygenCommentCache = ""
            return
        elif (self.braceDepth > len(self.nameSpaces) + 1):
            if (debug): print "line ",lineno()
            self.nameStack = []
            doxygenCommentCache = ""
            return
        elif is_enum_namestack(self.nameStack):
            if (debug): print "line ",lineno()
            #elif self.nameStack[0] == "enum":
            self.evaluate_enum_stack()
        elif ('(' in self.nameStack):
            if (debug): print "line ",lineno()
            self.evaluate_method_stack()
        else:
            if (debug): print "line ",lineno()
            self.evaluate_property_stack()
        self.nameStack = []
        doxygenCommentCache = ""
    
    def evaluate_class_stack(self):
        """Create a Class out of the name stack (but not its parts)"""
        #dont support sub classes today
        if self.braceDepth != len(self.nameSpaces):
            return
        newClass = CppClass(self.nameStack)
        if len(newClass.keys()):
            self.curClass = newClass["name"]
            self.classes[self.curClass] = newClass
        else:
            self.curClass = ""
        newClass["namespace"] = self.cur_namespace()

    def evaluate_method_stack(self):
        """Create a method out of the name stack"""
        newMethod = CppMethod(self.nameStack, self.curClass)
        if len(newMethod.keys()):
            self.classes[self.curClass]["methods"][self.curAccessSpecifier].append(newMethod)
    
    def evaluate_property_stack(self):
        """Create a Property out of the name stack"""
        newVar = CppVariable(self.nameStack)
        if len(newVar.keys()):
            self.classes[self.curClass]["properties"][self.curAccessSpecifier].append(newVar)

    def evaluate_enum_stack(self):
        """Create an Enum out of the name stack"""
        newEnum = CppEnum(self.nameStack)
        if len(newEnum.keys()):
            if len(self.curClass):
                newEnum["namespace"] = self.cur_namespace()
                self.classes[self.curClass]["enums"][self.curAccessSpecifier].append(newEnum)
            else:
                newEnum["namespace"] = self.cur_namespace()                            
#                print "Adding global enum"
                self.enums.append(newEnum)
            #This enum has instances, turn them into properties
            if newEnum.has_key("instances"):
                instanceType = "enum"
                if newEnum.has_key("name"):
                    instanceType = newEnum["name"]
                for instance in newEnum["instances"]:
                    self.nameStack = [instanceType,  instance]
                    self.evaluate_property_stack()
                del newEnum["instances"]

    def cur_namespace(self, add_double_colon = False):
        rtn = ""
        i = 0
        while i < len(self.nameSpaces):
            rtn += self.nameSpaces[i]
            if add_double_colon or i < len(self.nameSpaces) - 1:
                rtn += "::"
            i+=1
        return rtn
    
    
    def __repr__(self):
        rtn = ""
        for className in self.classes.keys():
            rtn += repr(self.classes[className])
        return rtn
