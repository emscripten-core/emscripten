#!/usr/bin/env python3
# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import json
import os
import shutil
import sys
import tempfile
import time

profiler_logs_path = os.path.join(tempfile.gettempdir(), 'emscripten_toolchain_profiler_logs')

OUTFILE = 'emprofile.' + time.strftime('%Y%m%d_%H%M')
for i in range(len(sys.argv)):
  arg = sys.argv[i]
  if arg.startswith('--outfile=') or arg.startswith('-o='):
    OUTFILE = arg.split('=', 1)[1].strip().replace('.html', '')
    sys.argv[i] = ''
  elif arg == '-o':
    OUTFILE = sys.argv[i + 1].strip().replace('.html', '')
    sys.argv[i] = sys.argv[i + 1] = ''


# Deletes all previously captured log files to make room for a new clean run.
def delete_profiler_logs():
  try:
    shutil.rmtree(profiler_logs_path)
  except IOError:
    pass


def list_files_in_directory(d):
  files = []
  try:
    items = os.listdir(d)
    for i in items:
      f = os.path.join(d, i)
      if os.path.isfile(f):
        files += [f]
    return files
  except IOError:
    return []


def create_profiling_graph():
  log_files = [f for f in list_files_in_directory(profiler_logs_path) if 'toolchain_profiler.pid_' in f]

  all_results = []
  if len(log_files):
    print('Processing ' + str(len(log_files)) + ' profile log files in "' + profiler_logs_path + '"...')
  for f in log_files:
    try:
      json_data = open(f, 'r').read()
      if len(json_data.strip()) == 0:
        continue
      lines = json_data.split('\n')
      lines = [x for x in lines if x != '[' and x != ']' and x != ',' and len(x.strip())]
      lines = [(x + ',') if not x.endswith(',') else x for x in lines]
      lines[-1] = lines[-1][:-1]
      json_data = '[' + '\n'.join(lines) + ']'
      all_results += json.loads(json_data)
    except Exception as e:
      print(str(e), file=sys.stderr)
      print('Failed to parse JSON file "' + f + '"!', file=sys.stderr)
      sys.exit(1)
  if len(all_results) == 0:
    print('No profiler logs were found in path "' + profiler_logs_path + '". Try setting the environment variable EMPROFILE=1 and run some emcc commands, and then rerun "emprofile" again.')
    return

  all_results.sort(key=lambda x: x['time'])

  emprofile_json_data = json.dumps(all_results, indent=2)

  html_file = OUTFILE + '.html'
  html_contents = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'toolchain_profiler.results_template.html'), 'r').read().replace('{{{ emprofile_json_data }}}', emprofile_json_data)
  open(html_file, 'w').write(html_contents)
  print('Wrote "' + html_file + '"')


if '--help' in sys.argv:
  print('''Usage:
       emprofile.py --clear (or -c)
         Deletes all previously recorded profiling log files.
         Use this to abort/drop any previously collected
         profiling data for a new profiling run.

       emprofile.py [--no-clear]
         Draws a graph from all recorded profiling log files,
         and deletes the recorded profiling files, unless
         --no-clear is also passed.

Optional parameters:

        --outfile=x.html (or -o=x.html)
          Specifies the name of the results file to generate.
''')
  sys.exit(1)


if '--reset' in sys.argv or '--clear' in sys.argv or '-c' in sys.argv:
  delete_profiler_logs()
else:
  create_profiling_graph()
  if '--no-clear' not in sys.argv:
    delete_profiler_logs()
