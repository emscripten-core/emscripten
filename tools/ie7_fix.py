'''
Simple tool to replace string lastchar from array access [] to charAt, for the purpose of IE7 support
'''
import os, sys, re

inputFilename = sys.argv[1];
outputFilename = sys.argv[2];

inputFiledata = open(inputFilename).read()
outputFile = open(outputFilename, "w")

outputFile.write(re.sub('type\[type.length - 1\] ===? "\*"', 'type.charAt(type.length - 1) == "*"', inputFiledata))

outputFile.close()
