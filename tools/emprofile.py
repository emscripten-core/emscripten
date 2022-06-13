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
from pathlib import Path


profiler_logs_path = os.path.join(tempfile.gettempdir(), 'emscripten_toolchain_profiler_logs')


# Deletes all previously captured log files to make room for a new clean run.
def delete_profiler_logs():
  if os.path.exists(profiler_logs_path):
    shutil.rmtree(profiler_logs_path)


def list_files_in_directory(d):
  files = []
  if os.path.exists(d):
    for i in os.listdir(d):
      f = os.path.join(d, i)
      if os.path.isfile(f):
        files.append(f)
  return files


def create_profiling_graph(outfile):
  log_files = [f for f in list_files_in_directory(profiler_logs_path) if 'toolchain_profiler.pid_' in f]

  all_results = []
  if len(log_files):
    print(f'Processing {len(log_files)} profile log files in {profiler_logs_path}...')
  for f in log_files:
    print(f'Processing: {f}')
    json_data = Path(f).read_text()
    if len(json_data.strip()) == 0:
      continue
    lines = json_data.split('\n')
    lines = [x for x in lines if x != '[' and x != ']' and x != ',' and len(x.strip())]
    lines = [(x + ',') if not x.endswith(',') else x for x in lines]
    lines[-1] = lines[-1][:-1]
    json_data = '[' + '\n'.join(lines) + ']'
    try:
      all_results += json.loads(json_data)
    except json.JSONDecodeError as e:
      print(str(e), file=sys.stderr)
      print('Failed to parse JSON file "' + f + '"!', file=sys.stderr)
      return 1
  if len(all_results) == 0:
    print(f'No profiler logs were found in path: ${profiler_logs_path}.\nTry setting the environment variable EMPROFILE=1 and run some emcc commands, then re-run "emprofile.py --graph".', file=sys.stderr)
    return 1

  all_results.sort(key=lambda x: x['time'])

  emprofile_json_data = json.dumps(all_results, indent=2)

  html_file = outfile + '.html'
  html_contents = Path(os.path.dirname(os.path.realpath(__file__)), 'toolchain_profiler.results_template.html').read_text().replace('{{{ emprofile_json_data }}}', emprofile_json_data)
  Path(html_file).write_text(html_contents)
  print(f'Wrote "{html_file}"')
  return 0


def main(args):
  if '--help' in args:
    print('''\
Usage:
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
    return 0

  if '--reset' in args or '--clear' in args or '-c' in args:
    delete_profiler_logs()
    return 0
  else:
    outfile = 'toolchain_profiler.results_' + time.strftime('%Y%m%d_%H%M')
    for i, arg in enumerate(args):
      if arg.startswith('--outfile=') or arg.startswith('-o='):
        outfile = arg.split('=', 1)[1].strip().replace('.html', '')
      elif arg == '-o':
        outfile = args[i + 1].strip().replace('.html', '')
    if create_profiling_graph(outfile):
      return 1
    if '--no-clear' not in args:
      delete_profiler_logs()

  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
