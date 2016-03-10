
import os, sys, subprocess, multiprocessing, re, string, json, shutil, logging, traceback
import shared
from js_optimizer import *

DUPLICATE_FUNCTION_ELIMINATOR = path_from_root('tools', 'eliminate-duplicate-functions.js')

def process_shell(js, js_engine, shell, equivalentfn_hash_info=None):
  suffix = '.eliminatedupes'

  temp_file = temp_files.get(suffix + '.js').name
  f = open(temp_file, 'w')
  f.write(shell)
  f.write('\n')

  f.write(equivalentfn_hash_info)
  f.close()

  (output,error) = subprocess.Popen(js_engine +
      [DUPLICATE_FUNCTION_ELIMINATOR, temp_file, '--use-hash-info', '--no-minimize-whitespace'],
      stdout=subprocess.PIPE,stderr=subprocess.PIPE).communicate()
  assert len(output) > 0
  assert len(error) == 0

  return output

def run_on_chunk(command):
  try:
    file_suffix = '.js'
    index = command.index(DUPLICATE_FUNCTION_ELIMINATOR)
    filename = command[index + 1]

    if '--gen-hash-info' in command:
      file_suffix = '.json'

    if os.environ.get('EMCC_SAVE_OPT_TEMP') and os.environ.get('EMCC_SAVE_OPT_TEMP') != '0':
      saved = 'save_' + os.path.basename(filename)
      while os.path.exists(saved): saved = 'input' + str(int(saved.replace('input', '').replace('.txt', ''))+1) + '.txt'
      print >> sys.stderr, 'running DFE command', ' '.join(map(lambda c: c if c != filename else saved, command))
      shutil.copyfile(filename, os.path.join(shared.get_emscripten_temp_dir(), saved))

    if shared.EM_BUILD_VERBOSE_LEVEL >= 3: print >> sys.stderr, 'run_on_chunk: ' + str(command)

    proc = subprocess.Popen(command, stdout=subprocess.PIPE)
    output = proc.communicate()[0]
    assert proc.returncode == 0, 'Error in optimizer (return code ' + str(proc.returncode) + '): ' + output
    assert len(output) > 0 and not output.startswith('Assertion failed'), 'Error in optimizer: ' + output
    filename = temp_files.get(os.path.basename(filename) + '.jo' + file_suffix).name

    # Important to write out in binary mode, because the data we are writing contains Windows line endings '\r\n' because it was PIPED from console.
    # Otherwise writing \r\n to ascii mode file will result in Windows amplifying \n to \r\n, generating bad \r\r\n line endings.
    f = open(filename, 'wb')
    f.write(output)
    f.close()
    if DEBUG and not shared.WINDOWS: print >> sys.stderr, '.' # Skip debug progress indicator on Windows, since it doesn't buffer well with multiple threads printing to console.
    return filename
  except KeyboardInterrupt:
    # avoid throwing keyboard interrupts from a child process
    raise Exception()
  except (TypeError, ValueError) as e:
    formatted_lines = traceback.format_exc().splitlines()

    print >> sys.stderr, ">>>>>>>>>>>>>>>>>"
    for formatted_line in formatted_lines:
        print >> sys.stderr, formatted_line
    print >> sys.stderr, "<<<<<<<<<<<<<<<<<"

    raise

def dump_equivalent_functions(passed_in_filename, global_data):
  # Represents the sets of equivalent functions for the passed in filename
  equivalent_fn_info = {}
  equivalent_fn_json_file = passed_in_filename + ".equivalent_functions.json"

  # If we are running more than one pass, then we want to merge
  # all the hash infos into one
  if os.path.isfile(equivalent_fn_json_file):
    print >> sys.stderr, "Merging data from current pass for {} into {}".format(passed_in_filename, equivalent_fn_json_file)
    with open(equivalent_fn_json_file) as data_file:
      equivalent_fn_info = json.load(data_file)
  else:
    print >> sys.stderr, "Writing equivalent functions for {} to {}".format(passed_in_filename, equivalent_fn_json_file)

  # Merge the global data's fn_hash_to_fn_name structure into
  # the equivalent function info hash.
  for fn_hash, fn_names in global_data['fn_hash_to_fn_name'].iteritems():
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
      for fn_hash, fn_names in data['fn_hash_to_fn_name'].iteritems():
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
      for variable, value in data['variable_names'].iteritems():
        if variable not in global_data['variable_names']:
            global_data['variable_names'][variable] = value

  variable_names = global_data['variable_names']

  # Lets generate the equivalent function hash from the global data set
  equivalent_fn_hash = {}
  for fn_hash, fn_names in global_data['fn_hash_to_fn_name'].iteritems():
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
  f.write('// ' + json.dumps(equivalent_fn_hash, separators=(',',':')))

# gen_hash_info is used to determine whether we are generating
# the global set of function implementation hashes. If set to
# False, we assume that we have to use the global hash info to
# reduce the set of duplicate functions
def run_on_js(filename, gen_hash_info=False):
  js_engine=shared.NODE_JS

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
    asm_shell = js[start_asm + len(start_asm_marker):start_funcs + len(start_funcs_marker)] + '''
EMSCRIPTEN_FUNCS();
''' + js[end_funcs + len(end_funcs_marker):end_asm + len(end_asm_marker)]
    js = js[start_funcs + len(start_funcs_marker):end_funcs]

    # we assume there is a maximum of one new name per line
    asm_shell_pre, asm_shell_post = process_shell(js, js_engine, asm_shell, equivalentfn_hash_info).split('EMSCRIPTEN_FUNCS();');
    asm_shell_post = asm_shell_post.replace('});', '})');
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
  cores = int(os.environ.get('EMCC_CORES') or multiprocessing.cpu_count())

  intended_num_chunks = int(round(cores * NUM_CHUNKS_PER_CORE))
  chunk_size = min(MAX_CHUNK_SIZE, max(MIN_CHUNK_SIZE, total_size / intended_num_chunks))
  chunks = shared.chunkify(funcs, chunk_size)

  chunks = filter(lambda chunk: len(chunk) > 0, chunks)
  if DEBUG and len(chunks) > 0: print >> sys.stderr, 'chunkification: num funcs:', len(funcs), 'actual num chunks:', len(chunks), 'chunk size range:', max(map(len, chunks)), '-', min(map(len, chunks))
  funcs = None

  if len(chunks) > 0:
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
  if len(filenames) > 0:
    commands = map(lambda filename: js_engine + [DUPLICATE_FUNCTION_ELIMINATOR, filename, '--gen-hash-info' if gen_hash_info else '--use-hash-info', '--no-minimize-whitespace'], filenames)

    if DEBUG and commands is not None:
      print >> sys.stderr, [' '.join(command if command is not None else '(null)') for command in commands]

    cores = min(cores, len(filenames))
    if len(chunks) > 1 and cores >= 2:
      # We can parallelize
      if DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks, using %d cores  (total: %.2f MB)' % (len(chunks), cores, total_size/(1024*1024.))
      pool = multiprocessing.Pool(processes=cores)
      filenames = pool.map(run_on_chunk, commands, chunksize=1)
      pool.terminate()
      pool.join()
    else:
      # We can't parallize, but still break into chunks to avoid uglify/node memory issues
      if len(chunks) > 1 and DEBUG: print >> sys.stderr, 'splitting up js optimization into %d chunks' % (len(chunks))
      filenames = [run_on_chunk(command) for command in commands]
  else:
    filenames = []

  json_files = []

  # We're going to be coalescing the files back at the end
  # Just replace the file list with the ones provided in
  # the command list - and save off the generated Json
  if gen_hash_info:
    json_files = filenames[:]
    filenames = old_filenames[:]

  for filename in filenames: temp_files.note(filename)

  filename += '.jo.js'
  f = open(filename, 'w')
  f.write(pre);
  pre = None

  # sort functions by size, to make diffing easier and to improve aot times
  funcses = []
  for out_file in filenames:
    funcses.append(split_funcs(open(out_file).read(), False))
  funcs = [item for sublist in funcses for item in sublist]
  funcses = None
  def sorter(x, y):
    diff = len(y[1]) - len(x[1])
    if diff != 0: return diff
    if x[0] < y[0]: return 1
    elif x[0] > y[0]: return -1
    return 0
  if not os.environ.get('EMCC_NO_OPT_SORT'):
    funcs.sort(sorter)

  for func in funcs:
    f.write(func[1])
  funcs = None

  f.write('\n')
  f.write(post);
  # No need to write suffix: if there was one, it is inside post which exists when suffix is there
  f.write('\n')

  if gen_hash_info and len(json_files) > 0:
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

    print >> sys.stderr, "Copying {} to {}".format(file_to_process, destinationFile)
    shutil.copyfile(file_to_process, destinationFile)

def get_func_names(javascript_file):
  func_names = []
  start_tok = "// EMSCRIPTEN_START_FUNCS"
  end_tok = "// EMSCRIPTEN_END_FUNCS"
  start_off = 0
  end_off = 0

  with open (javascript_file, 'rt') as fin:
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
      print >> sys.stderr, "Deleting old json: " + equivalent_fn_json_file
      os.remove(equivalent_fn_json_file)

    old_funcs = get_func_names(file_name)

  for pass_num in range(shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS_PASSES):
    if DEBUG: print >> sys.stderr, "[PASS {}]: eliminating duplicate functions in: {}.".format(pass_num, file_name)

    # Generate the JSON for the equivalent hash first
    processed_file = run_on_js(filename=file_name, gen_hash_info=True)

    save_temp_file(processed_file)

    # Use the hash to reduce the JS file
    final_file = run_on_js(filename=processed_file, gen_hash_info=False)

    save_temp_file(final_file)

    shared.safe_move(final_file, file_name)

  if shared.Settings.ELIMINATE_DUPLICATE_FUNCTIONS_DUMP_EQUIVALENT_FUNCTIONS != 0:
    new_funcs = get_func_names(file_name)

    eliminated_funcs_file = file_name + ".eliminated_functions.json"
    print >> sys.stderr, "Writing eliminated functions to file: {}".format(eliminated_funcs_file)

    with open(eliminated_funcs_file, 'w') as fout:
      eliminated_functions = list(set(old_funcs)-set(new_funcs))
      eliminated_functions.sort()
      for eliminated_function in eliminated_functions:
        fout.write('{}\n'.format(eliminated_function))

def run(filename, js_engine=shared.NODE_JS):
  js_engine = shared.listify(js_engine)

  return temp_files.run_and_clean(lambda: eliminate_duplicate_funcs(filename))

if __name__ == '__main__':
  out = run(sys.argv[1], sys.argv[2:])

