#!/usr/bin/env python

import sys, shutil, os, json, tempfile, time

profiler_logs_path = os.path.join(tempfile.gettempdir(), 'emscripten_toolchain_profiler_logs')

# If set to 1, always generates the output file under the same filename and doesn't delete the temp data.
DEBUG_EMPROFILE_PY = 0

OUTFILE = 'toolchain_profiler.results_' + time.strftime('%Y%m%d_%H%M')
for arg in sys.argv:
  if arg.startswith('--outfile='):
    OUTFILE = arg.split('=')[1].strip().replace('.html', '')

# Deletes all previously captured log files to make room for a new clean run.
def delete_profiler_logs():
  try:
    shutil.rmtree(profiler_logs_path)
  except:
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
  except:
    return []

def create_profiling_graph():
  log_files = [f for f in list_files_in_directory(profiler_logs_path) if 'toolchain_profiler.pid_' in f]

  all_results = []
  if len(log_files) > 0:
    print 'Processing ' + str(len(log_files)) + ' profile log files in "' + profiler_logs_path + '"...'
  for f in log_files:
    try:
      json_data = open(f, 'r').read()
      lines = json_data.split('\n')
      lines = filter(lambda x: x != '[' and x != ']' and x != ',' and len(x.strip()) > 0, lines)
      lines = map(lambda x: (x + ',') if not x.endswith(',') else x, lines)
      lines[-1] = lines[-1][:-1]
      json_data = '[' + '\n'.join(lines) + ']'
      all_results += json.loads(json_data)
    except Exception, e:
      print >> sys.stderr, str(e)
      print >> sys.stderr, 'Failed to parse JSON file "' + f + '"!'
      sys.exit(1)
  if len(all_results) == 0:
    print 'No profiler logs were found in path "' + profiler_logs_path + '". Try setting the environment variable EM_PROFILE_TOOLCHAIN=1 and run some emcc commands, and then rerun "python emprofile.py --graph" again.'
    return

  all_results.sort(key=lambda x: x['time'])

  json_file = OUTFILE + '.json'
  open(json_file, 'w').write(json.dumps(all_results, indent=2))
  print 'Wrote "' + json_file + '"'

  html_file = OUTFILE + '.html'
  html_contents = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'toolchain_profiler.results_template.html'), 'r').read().replace('{{{results_log_file}}}', '"' + json_file + '"')
  open(html_file, 'w').write(html_contents)
  print 'Wrote "' + html_file + '"'

  if not DEBUG_EMPROFILE_PY:
    delete_profiler_logs()

if len(sys.argv) < 2:
  print '''Usage:
       emprofile.py --reset
         Deletes all previously recorded profiling log files.

       emprofile.py --graph
         Draws a graph from all recorded profiling log files.

Optional parameters:

        --outfile=x.html
          Specifies the name of the results file to generate.
'''
  sys.exit(1)

if '--reset' in sys.argv:
  delete_profiler_logs()
elif '--graph' in sys.argv:
  create_profiling_graph()
else:
  print 'Unknown command "' + sys.argv[1] + '"!'
  sys.exit(1)
