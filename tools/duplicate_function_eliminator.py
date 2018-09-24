# Copyright 2016 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from __future__ import print_function
import os
import sys
import subprocess
import re
import json
import shutil
import logging
import traceback

sys.path.insert(1, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools import shared
from tools.js_optimizer import DEBUG, temp_files, start_funcs_marker, end_funcs_marker, split_funcs, start_asm_marker, end_asm_marker
from tools.js_optimizer import MIN_CHUNK_SIZE, MAX_CHUNK_SIZE, NUM_CHUNKS_PER_CORE

DUPLICATE_FUNCTION_ELIMINATOR = shared.path_from_root('tools', 'eliminate-duplicate-functions.js')


def process_shell(js_engine, shell, equivalentfn_hash_info=None):
  suffix = '.eliminatedupes'

  with temp_files.get_file(suffix + '.js') as temp_file:
    f = open(temp_file, 'w')
    f.write(shell)
    f.write('\n')

    f.write(equivalentfn_hash_info)
    f.close()

    proc = shared.run_process(
        js_engine +
        [DUPLICATE_FUNCTION_ELIMINATOR, temp_file, '--use-hash-info', '--no-minimize-whitespace'],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  assert len(proc.stdout)
  assert len(proc.stderr) == 0

  return proc.stdout


def run_on_chunk(command):
  try:
    file_suffix = '.js'
    index = command.index(DUPLICATE_FUNCTION_ELIMINATOR)
    filename = command[index + 1]

    if '--gen-hash-info' in command:
      file_suffix = '.json'

    if os.environ.get('EMCC_SAVE_OPT_TEMP') and os.environ.get('EMCC_SAVE_OPT_TEMP') != '0':
      saved = 'save_' + os.path.basename(filename)
      while os.path.exists(saved):
        saved = 'input' + str(int(saved.replace('input', '').replace('.txt', '')) + 1) + '.txt'
      print('running DFE command', ' '.join([c if c != filename else saved for c in command]), file=sys.stderr)
      shutil.copyfile(filename, os.path.join(shared.get_emscripten_temp_dir(), saved))

    if shared.EM_BUILD_VERBOSE >= 3:
      print('run_on_chunk: ' + str(command), file=sys.stderr)

    proc = shared.run_process(command, stdout=subprocess.PIPE)
    output = proc.stdout
    assert proc.returncode == 0, 'Error in optimizer (return code ' + str(proc.returncode) + '): ' + output
    assert len(output) and not output.startswith('Assertion failed'), 'Error in optimizer: ' + output
    filename = temp_files.get(os.path.basename(filename) + '.dfjo' + file_suffix).name

    f = open(filename, 'w')
    f.write(output)
    f.close()
    if DEBUG and not shared.WINDOWS:
      print('.', file=sys.stderr) # Skip debug progress indicator on Windows, since it doesn't buffer well with multiple threads printing to console.
    return filename
  except KeyboardInterrupt:
    # avoid throwing keyboard interrupts from a child process
    raise Exception()
  except (TypeError, ValueError):
    formatted_lines = traceback.format_exc().splitlines()

    print(">>>>>>>>>>>>>>>>>", file=sys.stderr)
    for formatted_line in formatted_lines:
        print(formatted_line, file=sys.stderr)
    print("<<<<<<<<<<<<<<<<<", file=sys.stderr)

    raise


def dump_equivalent_functions(passed_in_filename, global_data):
  # Represents the sets of equivalent functions for the passed in filename
  equivalent_fn_info = {}
  equivalent_fn_json_file = passed_in_filename + ".equivalent_functions.json"

  # If we are running more than one pass, then we want to merge
  # all the hash infos into one
  if os.path.isfile(equivalent_fn_json_file):
    print("Merging data from current pass for {} into {}".format(passed_in_filename, equivalent_fn_json_file), file=sys.stderr)
    with open(equivalent_fn_json_file) as data_file:
      equivalent_fn_info = json.load(data_file)
  else:
    print("Writing equivalent functions for {} to {}".format(passed_in_filename, equivalent_fn_json_file), file=sys.stderr)

  # Merge the global data's fn_hash_to_fn_name structure into
  # the equivalent function info hash.
  for fn_hash, fn_names in global_data['fn_hash_to_fn_name'].items():
    if fn_hash not in equivalent_fn_info:
      # Exclude single item arrays as they are of no use to us.
      if len(fn_names) > 1:
        equivalent_fn_info[fn_hash] = fn_names[:]
    else:
      for fn_name in fn_names:
        if fn_name not in equivalent_fn_info[fn_hash]:
          equivalent_fn_info[fn_hash].append(fn_name)

  with open(equivalent_fn_json_file, 'w') as fout:
    fout.write(json.dumps(equivalent_fn_info))


def write_equivalent_fn_hash_to_file(f, json_files, passed_in_filename):
  # Represents the aggregated info for all the json files passed in
  # Each json file contains info for one of the processed chunks
  global_data = {}
  global_data['fn_hash_to_fn_name'] = {}
  global_data['fn_hash_to_fn_body'] = {}
  global_data['variable_names'] = {}

  for json_file in json_files:
    with open(json_file) as data_file:
      data = json.load(data_file)

      # Merge the data's fn_hash_to_fn_name structure into
      # the global data hash.
      for fn_hash, fn_names in data['fn_hash_to_fn_name'].items():
        if fn_hash not in global_data['fn_hash_to_fn_name']:
            global_data['fn_hash_to_fn_name'][fn_hash] = fn_names[:]
            global_data['fn_hash_to_fn_body'][fn_hash] = data['fn_hash_to_fn_body'][fn_hash]
        else:
          assert(data['fn_hash_to_fn_body'][fn_hash] == global_data['fn_hash_to_fn_body'][fn_hash])

          for fn_name in fn_names:
            if fn_name not in global_data['fn_hash_to_fn_name'][fn_hash]:
              global_data['fn_hash_to_fn_name'][fn_hash].append(fn_name)

      # Merge the data's variable_names structure into
      # the global data hash.
      for variable, value in data['variable_names'].items():
        if variable not in global_data['variable_names']:
            global_data['variable_names'][variable] = value

  variable_names = global_data['variable_names']

  # Lets generate the equivalent function hash from the global data set
  equivalent_fn_hash = {}
  for fn_hash, fn_names in global_data['fn_hash_to_fn_name'].items():
    shortest_fn = None
    for fn_name in fn_names:
      if (fn_name not in variable_names) and (shortest_fn is None or (len(fn_name) < len(shortest_fn))):
        shortest_fn = fn_name

    if shortest_fn is not None:
      for fn_name in fn_names:
        if fn_name not in variable_names and fn_name != shortest_fn:
          equivalent_fn_hash[fn_name] = shortest_fn

  # Dump the sets of equivalent functions if the user desires it
  # This comes in handy for debugging
  if shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS_DUMP_EQUIVALENT_FUNCTIONS:
    dump_equivalent_functions(passed_in_filename, global_data)

  # Now write the equivalent function hash to the last line of the file
  f.write('// ' + json.dumps(equivalent_fn_hash, separators=(',', ':')))


# gen_hash_info is used to determine whether we are generating
# the global set of function implementation hashes. If set to
# False, we assume that we have to use the global hash info to
# reduce the set of duplicate functions
# Returns the filename of the processed JS file, which is expected to be
# deleted by the caller once done.
def run_on_js(filename, gen_hash_info=False):
  js_engine = shared.NODE_JS

  js = open(filename).read()
  if os.linesep != '\n':
    js = js.replace(os.linesep, '\n') # we assume \n in the splitting code

  equivalentfn_hash_info = None
  passed_in_filename = filename

  # Find markers
  start_funcs = js.find(start_funcs_marker)
  end_funcs = js.rfind(end_funcs_marker)

  if start_funcs < 0 or end_funcs < start_funcs:
    logging.critical('Invalid input file. Did not contain appropriate markers. (start_funcs: %s, end_funcs: %s)' % (start_funcs, end_funcs))
    sys.exit(1)

  if not gen_hash_info:
    equivalentfn_hash_info = js[js.rfind('//'):]

    start_asm = js.find(start_asm_marker)
    end_asm = js.rfind(end_asm_marker)
    assert (start_asm >= 0) == (end_asm >= 0)

    # We need to split out the asm shell as well, for minification
    pre = js[:start_asm + len(start_asm_marker)]
    post = js[end_asm:]
    asm_shell_pre = js[start_asm + len(start_asm_marker):start_funcs + len(start_funcs_marker)]
    # Prevent "uglify" from turning 0.0 into 0 in variables' initialization. To do this we first replace 0.0 with
    # ZERO$DOT$ZERO and then replace it back.
    asm_shell_pre = re.sub(r'(\S+\s*=\s*)0\.0', r'\1ZERO$DOT$ZERO', asm_shell_pre)
    asm_shell_post = js[end_funcs + len(end_funcs_marker):end_asm + len(end_asm_marker)]
    asm_shell = asm_shell_pre + '\nEMSCRIPTEN_FUNCS();\n' + asm_shell_post
    js = js[start_funcs + len(start_funcs_marker):end_funcs]

    # we assume there is a maximum of one new name per line
    asm_shell_pre, asm_shell_post = process_shell(js_engine, asm_shell, equivalentfn_hash_info).split('EMSCRIPTEN_FUNCS();')
    asm_shell_pre = re.sub(r'(\S+\s*=\s*)ZERO\$DOT\$ZERO', r'\g<1>0.0', asm_shell_pre)
    asm_shell_post = asm_shell_post.replace('});', '})')
    pre += asm_shell_pre + '\n' + start_funcs_marker
    post = end_funcs_marker + asm_shell_post + post

    if not gen_hash_info:
      # We don't need the extra info at the end
      post = post[:post.rfind('//')].strip()
  else:
    pre = js[:start_funcs + len(start_funcs_marker)]
    post = js[end_funcs + len(end_funcs_marker):]
    js = js[start_funcs + len(start_funcs_marker):end_funcs]
    post = end_funcs_marker + post

  total_size = len(js)
  funcs = split_funcs(js, False)

  js = None

  # if we are making source maps, we want our debug numbering to start from the
  # top of the file, so avoid breaking the JS into chunks
  cores = shared.Building.get_num_cores()

  intended_num_chunks = int(round(cores * NUM_CHUNKS_PER_CORE))
  chunk_size = min(MAX_CHUNK_SIZE, max(MIN_CHUNK_SIZE, total_size / intended_num_chunks))
  chunks = shared.chunkify(funcs, chunk_size)

  chunks = [chunk for chunk in chunks if len(chunk)]
  if DEBUG and len(chunks):
    print('chunkification: num funcs:', len(funcs), 'actual num chunks:', len(chunks), 'chunk size range:', max(map(len, chunks)), '-', min(map(len, chunks)), file=sys.stderr)
  funcs = None

  if len(chunks):
    def write_chunk(chunk, i):
      temp_file = temp_files.get('.jsfunc_%d.js' % i).name
      f = open(temp_file, 'w')
      f.write(chunk)

      if not gen_hash_info:
        f.write('\n')
        f.write(equivalentfn_hash_info)
      f.close()
      return temp_file
    filenames = [write_chunk(chunks[i], i) for i in range(len(chunks))]
  else:
    filenames = []

  old_filenames = filenames[:]
  if len(filenames):
    commands = [js_engine + [DUPLICATE_FUNCTION_ELIMINATOR, f, '--gen-hash-info' if gen_hash_info else '--use-hash-info', '--no-minimize-whitespace'] for f in filenames]

    if DEBUG and commands is not None:
      print([' '.join(command if command is not None else '(null)') for command in commands], file=sys.stderr)

    cores = min(cores, len(filenames))
    if len(chunks) > 1 and cores >= 2:
      # We can parallelize
      if DEBUG:
        print('splitting up js optimization into %d chunks, using %d cores  (total: %.2f MB)' % (len(chunks), cores, total_size / (1024 * 1024.)), file=sys.stderr)
      pool = shared.Building.get_multiprocessing_pool()
      filenames = pool.map(run_on_chunk, commands, chunksize=1)
    else:
      # We can't parallize, but still break into chunks to avoid uglify/node memory issues
      if len(chunks) > 1 and DEBUG:
        print('splitting up js optimization into %d chunks' % (len(chunks)), file=sys.stderr)
      filenames = [run_on_chunk(command) for command in commands]
  else:
    filenames = []

  # we create temp files in the child threads, clean them up here when we are done
  for filename in filenames:
    temp_files.note(filename)

  json_files = []

  # We're going to be coalescing the files back at the end
  # Just replace the file list with the ones provided in
  # the command list - and save off the generated Json
  if gen_hash_info:
    json_files = filenames[:]
    filenames = old_filenames[:]

  for filename in filenames:
    temp_files.note(filename)

  filename += '.jo.js'
  f = open(filename, 'w')
  f.write(pre)
  pre = None

  # sort functions by size, to make diffing easier and to improve aot times
  funcses = []
  for out_file in filenames:
    funcses.append(split_funcs(open(out_file).read(), False))
  funcs = [item for sublist in funcses for item in sublist]
  funcses = None
  if not os.environ.get('EMCC_NO_OPT_SORT'):
    funcs.sort(key=lambda x: (len(x[1]), x[0]), reverse=True)

  for func in funcs:
    f.write(func[1])
  funcs = None

  f.write('\n')
  f.write(post)
  # No need to write suffix: if there was one, it is inside post which exists when suffix is there
  f.write('\n')

  if gen_hash_info and len(json_files):
    write_equivalent_fn_hash_to_file(f, json_files, passed_in_filename)
  f.close()

  return filename


def save_temp_file(file_to_process):
  if os.environ.get('EMSCRIPTEN_SAVE_TEMP_FILES') and os.environ.get('EMSCRIPTEN_TEMP_FILES_DIR'):
    destinationFile = file_to_process

    temp_dir_name = os.environ.get('TEMP_DIR')
    destinationFile = destinationFile.replace(temp_dir_name, os.environ.get('EMSCRIPTEN_TEMP_FILES_DIR'))

    if not os.path.exists(os.path.dirname(destinationFile)):
      os.makedirs(os.path.dirname(destinationFile))

    print("Copying {} to {}".format(file_to_process, destinationFile), file=sys.stderr)
    shutil.copyfile(file_to_process, destinationFile)


def get_func_names(javascript_file):
  func_names = []
  start_tok = "// EMSCRIPTEN_START_FUNCS"
  end_tok = "// EMSCRIPTEN_END_FUNCS"
  start_off = 0
  end_off = 0

  with open(javascript_file, 'rt') as fin:
    blob = "".join(fin.readlines())
    start_off = blob.find(start_tok) + len(start_tok)
    end_off = blob.find(end_tok)
    asm_chunk = blob[start_off:end_off]

    for match in re.finditer('function (\S+?)\s*\(', asm_chunk):
      func_names.append(match.groups(1)[0])

  return func_names


def eliminate_duplicate_funcs(file_name):
  if shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS_DUMP_EQUIVALENT_FUNCTIONS != 0:
    # Remove previous log file if it exists
    equivalent_fn_json_file = file_name + ".equivalent_functions.json"
    if os.path.isfile(equivalent_fn_json_file):
      print("Deleting old json: " + equivalent_fn_json_file, file=sys.stderr)
      os.remove(equivalent_fn_json_file)

    old_funcs = get_func_names(file_name)

  for pass_num in range(shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS_PASSES):
    if DEBUG:
      print("[PASS {}]: eliminating duplicate functions in: {}.".format(pass_num, file_name), file=sys.stderr)

    # Generate the JSON for the equivalent hash first
    processed_file = run_on_js(filename=file_name, gen_hash_info=True)
    try:
      save_temp_file(processed_file)
      # Use the hash to reduce the JS file
      final_file = run_on_js(filename=processed_file, gen_hash_info=False)
    finally:
      os.remove(processed_file)

    save_temp_file(final_file)

    shared.safe_move(final_file, file_name)

  if shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS_DUMP_EQUIVALENT_FUNCTIONS != 0:
    new_funcs = get_func_names(file_name)

    eliminated_funcs_file = file_name + ".eliminated_functions.json"
    print("Writing eliminated functions to file: {}".format(eliminated_funcs_file), file=sys.stderr)

    with open(eliminated_funcs_file, 'w') as fout:
      eliminated_functions = list(set(old_funcs) - set(new_funcs))
      eliminated_functions.sort()
      for eliminated_function in eliminated_functions:
        fout.write('{}\n'.format(eliminated_function))


def run(filename, js_engine=shared.NODE_JS):
  js_engine = shared.listify(js_engine)

  return temp_files.run_and_clean(lambda: eliminate_duplicate_funcs(filename))


if __name__ == '__main__':
  run(sys.argv[1], sys.argv[2:])
  sys.exit(0)
