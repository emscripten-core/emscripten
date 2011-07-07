#!/usr/bin/python
import sys
sys.path = ["../"] + sys.path
import CppHeaderParser
try:
    cppHeader = CppHeaderParser.CppHeader("SampleClass.h")
except CppHeaderParser.CppParseError,  e:
    print e
    sys.exit(1)

print "CppHeaderParser view of %s"%cppHeader

sampleClass = cppHeader.classes["SampleClass"]
print "Number of public methods %d"%(len(sampleClass["methods"]["public"]))
print "Number of private properties %d"%(len(sampleClass["properties"]["private"]))
meth3 = [m for m in sampleClass["methods"]["public"] if m["name"] == "meth3"][0] #get meth3
meth3ParamTypes = [t["type"] for t in meth3["parameters"]] #get meth3s parameters
print "Parameter Types for public method meth3 %s"%(meth3ParamTypes)

print "\nReturn type for meth1:"
print cppHeader.classes["SampleClass"]["methods"]["public"][1]["rtnType"]

print "\nDoxygen for meth2:"
print cppHeader.classes["SampleClass"]["methods"]["public"][2]["doxygen"]

print "\nParameters for meth3:"
print cppHeader.classes["SampleClass"]["methods"]["public"][3]["parameters"]

print "\nDoxygen for meth4:"
print cppHeader.classes["SampleClass"]["methods"]["public"][4]["doxygen"]

print "\nReturn type for meth5:"
print cppHeader.classes["SampleClass"]["methods"]["private"][0]["rtnType"]

print "\nDoxygen type for prop1:"
print cppHeader.classes["SampleClass"]["properties"]["private"][0]["doxygen"]

print "\nType for prop5:"
print cppHeader.classes["SampleClass"]["properties"]["private"][1]["type"]

print "\nNamespace for AlphaClass is:"
print cppHeader.classes["AlphaClass"]["namespace"]

print "\nReturn type for alphaMethod is:"
print cppHeader.classes["AlphaClass"]["methods"]["public"][0]["rtnType"]

print "\nNamespace for OmegaClass is:"
print cppHeader.classes["OmegaClass"]["namespace"]

print "\nType for omegaString is:"
print cppHeader.classes["AlphaClass"]["properties"]["public"][0]["type"]

# by hart
print 'global typedefs:'
print cppHeader.typedefs

print 'order of classes:'
for cls in cppHeader.classes_order:
	print '\t', cls['name']

print 'order of typedefs:'
for name in cppHeader.typedefs_order:
	print '\t', name

print cppHeader.classes['ThetaClass']
print '_'*80
print cppHeader.classes['ThetaClass2']

print cppHeader.structs

print cppHeader.namespaces

print cppHeader.classes['FixMe']

