#!/usr/bin/env python3
# Copyright 2020 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import sys

'''
Example of parsed format:

test_base64 (test_benchmark.benchmark) ...
Running benchmarker: NativeBenchmarker: clang
        clang: mean: 0.731 (+-0.012) secs  median: 0.727  range: 0.715-0.746  (noise: 1.589%)  (5 runs)
Running benchmarker: NativeBenchmarker: gcc
[..]
'''

benchmark = ''
benchmarker = ''
matrix = []

for line in open(sys.argv[1]).readlines():
  line = line.strip()
  if line.startswith('test_'):
    benchmark = line.split(' ')[0][5:]
    # print('benchmark:', benchmark)
    if len(matrix) == 0:
      # the first line has [(free space), benchmarker name 1, ..]
      matrix += [[' ']]
    # other lines have [benchmark name, result 1, ..]
    matrix += [[benchmark]]
  if line.startswith('Running benchmarker'):
    benchmarker = line.split(':')[-1].strip()
    if benchmarker not in matrix[0]:
      matrix[0] += [benchmarker]
    # print('benchmarker:', benchmarker)
  elif line.startswith(benchmarker + ':'):
    parts = line.strip().split(' ')
    mean = float(parts[2])
    median = float(parts[7])
    noise = float(parts[13][:-2])
    if noise > 5:
      print('warning: noisy! (%s: %f%%)' % (benchmark + '.' + benchmarker, noise))
    if abs(mean - median) / mean > 0.05:
      print('warning: mean and median diverge! (%s: %f vs %f)' % (benchmark + '.' + benchmarker, mean, median))
    # print(benchmark, benchmarker, mean, median, noise)
    matrix[-1] += [median]

# normalize results
for line in matrix[1:]:
  if len(line) >= 2:
    base = line[1]
    for i in range(1, len(line)):
      line[i] = line[i] / base

# filter results
result = []
for line in matrix:
  if len(line) != len(matrix[0]):
    print('warning: not enough results, skipping line:', line[0])
  else:
    result += ['\t'.join([str(x) for x in line])]

# print results
print()
print('\n'.join(result))
print()
