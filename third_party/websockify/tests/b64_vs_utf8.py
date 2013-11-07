#!/usr/bin/env python

from base64 import b64encode, b64decode
from codecs import (utf_8_encode, utf_8_decode,
                    latin_1_encode, latin_1_decode)
import random, time

buf_len = 10000
iterations = 10000

print "Generating random input buffer"
r = random.Random()
buf = "".join([chr(r.randint(0, 255)) for i in range(buf_len)])

tests = {'UTF8 encode': lambda: utf_8_encode(unicode(buf, 'latin-1'))[0],
         'B64 encode': lambda: b64encode(buf)}
utf8_buf = tests['UTF8 encode']()
b64_buf = tests['B64 encode']()
tests.update({'UTF8 decode': lambda: latin_1_encode(utf_8_decode(utf8_buf)[0])[0],
              'B64 decode': lambda: b64decode(b64_buf)})

print "Running tests"
for test in 'UTF8 encode', 'B64 encode', 'UTF8 decode', 'B64 decode':
    start = time.time()
    for i in range(iterations):
        res_buf = tests[test]()
    print "%s took %s seconds (result size %s)" % (
            test, (time.time() -  start), len(res_buf))

