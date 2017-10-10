'''
Tool to find or compare big functions in a js or ll file
'''
from __future__ import print_function
import os, sys

def humanbytes(nbytes):
    if nbytes > 9*1024*1024:
        return '{}MB'.format(nbytes/1024/1024)
    elif nbytes > 9*1024:
        return '{}KB'.format(nbytes/1024)
    else:
        return '{}B'.format(nbytes)


def processfile(filename):
    start = None
    curr = None
    nbytes = None
    data = {}
    for i, line in enumerate(open(filename)):
        if line.startswith(('function ', 'define ')) and '}' not in line:
            start = i
            curr = line
            nbytes = len(line)
        elif line.startswith('}') and curr:
            nlines = i - start
            data[curr] = (nlines, nbytes+1)
            curr = None
            start = None
        elif curr:
            nbytes += len(line)
    return data

def common_compare(data1, data2):
    fns1 = set(data1.keys())
    fns2 = set(data2.keys())
    commonfns = fns1.intersection(fns2)
    commonlinediff = 0
    commonbytediff = 0
    for fn in commonfns:
        d1 = data1[fn]
        d2 = data2[fn]
        commonlinediff += d2[0] - d1[0]
        commonbytediff += d2[1] - d1[1]
    linesword = 'more' if commonlinediff >= 0 else 'less'
    bytesword = 'more' if commonbytediff >= 0 else 'less'
    print('file 2 has {} lines {} than file 1 in {} common functions'.format(abs(commonlinediff), linesword, len(commonfns)))
    print('file 2 has {} {} than file 1 in {} common functions'.format(humanbytes(abs(commonbytediff)), bytesword, len(commonfns)))

def uniq_compare(data1, data2):
    fns1 = set(data1.keys())
    fns2 = set(data2.keys())
    uniqfns1 = fns1 - fns2
    uniqfns2 = fns2 - fns1
    uniqlines1 = 0
    uniqbytes1 = 0
    uniqlines2 = 0
    uniqbytes2 = 0
    for fn in uniqfns1:
        d = data1[fn]
        uniqlines1 += d[0]
        uniqbytes1 += d[1]
    for fn in uniqfns2:
        d = data2[fn]
        uniqlines2 += d[0]
        uniqbytes2 += d[1]
    uniqcountdiff = len(uniqfns2) - len(uniqfns1)
    assert len(fns2) - len(fns1) == uniqcountdiff
    uniqlinediff = uniqlines2 - uniqlines1
    uniqbytediff = uniqbytes2 - uniqbytes1
    countword = 'more' if uniqcountdiff >= 0 else 'less'
    linesword = 'more' if uniqlinediff >= 0 else 'less'
    bytesword = 'more' if uniqbytediff >= 0 else 'less'
    print('file 2 has {} functions {} than file 1 overall (unique: {} vs {})'.format(abs(uniqcountdiff), countword, len(uniqfns2), len(uniqfns1)))
    print('file 2 has {} lines {} than file 1 overall in unique functions'.format(abs(uniqlinediff), linesword))
    print('file 2 has {} {} than file 1 overall in unique functions'.format(humanbytes(abs(uniqbytediff)), bytesword))

def list_bigfuncs(data):
    data = data.items()
    data.sort(lambda (f1, d1), (f2, d2): d1[0] - d2[0])
    print(''.join(['%6d lines (%6s) : %s' % (d[0], humanbytes(d[1]), f) for f, d in data]))

def main():
    if len(sys.argv) < 2 or len(sys.argv) > 3 or sys.argv[1] == '--help':
        print('Usage:')
        print('    {} file1 - list functions in a file in ascending order of size'.format(sys.argv[0]))
        print('    {} file1 file2 - compare functions across two files'.format(sys.argv[0]))
        sys.exit(1)
    elif len(sys.argv) == 2:
        filename = sys.argv[1]
        data = processfile(filename)
        list_bigfuncs(data)
        sys.exit(0)
    elif len(sys.argv) == 3:
        filename1 = sys.argv[1]
        data1 = processfile(filename1)
        filename2 = sys.argv[2]
        data2 = processfile(filename2)
        uniq_compare(data1, data2)
        common_compare(data1, data2)
        sys.exit(0)
    assert False

main()
