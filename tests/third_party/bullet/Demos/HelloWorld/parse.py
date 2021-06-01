from __future__ import print_function
import math

data = '''
"frame 0, 6.58, 398" setup.js:8
"frame 1, 17.86, 563" setup.js:8
"frame 2, 32.26, 624" setup.js:8
"frame 3, 30.30, 666" setup.js:8
"frame 4, 27.78, 704" setup.js:8
"frame 5, 55.56, 757" setup.js:8
"frame 6, 55.56, 788" setup.js:8
"frame 7, 52.63, 823" setup.js:8
"frame 8, 47.62, 852" setup.js:8
"frame 9, 52.63, 912" setup.js:8
"frame 10 starting" frames.js:1
"frame 10 complete" frames.js:1
"frame 10, 45.45, 941" setup.js:8
"frame 11, 66.67, 983" setup.js:8
"frame 12, 62.50, 1006" setup.js:8
"frame 13, 66.67, 1063" setup.js:8
"frame 14, 71.43, 1084" setup.js:8
"frame 15, 66.67, 1099" setup.js:8
"frame 16, 62.50, 1130" setup.js:8
"frame 17, 71.43, 1160" setup.js:8
"frame 18, 58.82, 1188" setup.js:8
"frame 19, 71.43, 1224" setup.js:8
"frame 20 starting" frames.js:1
"frame 20 complete" frames.js:1
"frame 20, 40.00, 1239" setup.js:8
"frame 21, 47.62, 1279" setup.js:8
"frame 22, 71.43, 1306" setup.js:8
"frame 23, 66.67, 1342" setup.js:8
"frame 24, 41.67, 1363" setup.js:8
"frame 25, 71.43, 1405" setup.js:8
"frame 26, 52.63, 1427" setup.js:8
"frame 27, 71.43, 1464" setup.js:8
"frame 28, 71.43, 1484" setup.js:8
"frame 29, 66.67, 1516" setup.js:8
"frame 30 starting" frames.js:1
"frame 30 complete"
'''

total_time = 3000 # ms
bin_size = 50 # ms
bins = [0] * (total_time / bin_size)

for line in data.split('\n'):
  line = line.strip()
  if 'frame ' not in line:
    continue
  if 'starting' in line or 'complete' in line:
    continue
  line = line.replace(', ', ',').replace(' ', ',').replace('"', '')
  # print line
  _, i, fps, wall, __ = line.split(',')
  # print i, fps, wall
  fps = float(fps)
  wall = float(wall)
  pos = wall / bin_size
  start_bin = int(math.floor(pos))
  assert start_bin < len(bins)
  end_bin = int(math.ceil(pos))
  assert start_bin + 1 == end_bin
  frac = pos % 1
  old = bins[start_bin]
  bins[start_bin] = frac * old + (1 - frac) * fps # interpolate in this bin
  for i in range(end_bin, len(bins)): # fill in this fps for all future bins, we assume no degradation
    bins[i] = fps

for i in range(len(bins)):
  print(i * bin_size, bins[i])
