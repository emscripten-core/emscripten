import tempfile, os, sys, shlex
import shared

# Routes the given cmdline param list in args into a new response file and returns the filename to it.
# The returned filename has a suffix '.rsp'.
def create_response_file(args, directory):
  (response_fd, response_filename) = tempfile.mkstemp(prefix='emscripten_', suffix='.rsp', dir=directory, text=True)
  response_fd = os.fdopen(response_fd, "w")
  #print >> sys.stderr, "Creating response file '%s'" % response_filename
  args = map(lambda p: p.replace('\\', '\\\\').replace('"', '\\"'), args)
  response_fd.write('"' + '" "'.join(args) + '"')
  response_fd.close()
  
  # Register the created .rsp file to be automatically cleaned up once this process finishes, so that
  # caller does not have to remember to do it.
  shared.configuration.get_temp_files().note(response_filename)
  
  return response_filename

# Reads a response file, and returns the list of cmdline params found in the file.
# The parameter response_filename may start with '@'.
def read_response_file(response_filename):
  if response_filename.startswith('@'):
    response_filename = response_filename[1:]

  #print >> sys.stderr, "Using response file '%s'" % response_filename
  if not os.path.exists(response_filename):
    raise Exception("Response file '%s' not found!" % response_filename)

  response_fd = open(response_filename, 'r')
  args = response_fd.read()
  response_fd.close()
  args = shlex.split(args)
  return args
