import tempfile, os, sys, shlex
from tempfiles import try_delete

# Routes the given cmdline param list in args into a new .rsp file and returns the filename to it.
# The returned filename has '@' prepended to it already for convenience.
def create_response_file(args, directory):
  (response_fd, response_filename) = tempfile.mkstemp(prefix='emscripten_', suffix='.rsp', dir=directory, text=True)
  response_fd = os.fdopen(response_fd, "w")
  #print >> sys.stderr, "Creating response file '%s'" % response_filename
  args = map(lambda p: p.replace(' ', '').replace('\\', '\\\\').replace('"', '\\"'), args)
  response_fd.write(' '.join(args))
  response_fd.close
  return '@' + response_filename

# Reads and deletes a .rsp file, and returns the list of cmdline params found in the file.
def read_and_delete_response_file(response_filename):
  # Ensure safety so that this function can never accidentally delete any non-.rsp files if things go wrong!
  if not (response_filename.startswith('@') and response_filename.endswith('.rsp')):
    raise Exception("'%s' is not a valid response file name! Response file names must start with '@' and end in '.rsp'!" % response_filename)
  response_filename = response_filename[1:]

  #print >> sys.stderr, "Using response file '%s'" % response_filename
  if not os.path.exists(response_filename):
    raise Exception("Response file '%s' not found!" % response_filename)

  response_fd = open(response_filename, 'r')
  args = response_fd.read()
  response_fd.close()
  try_delete(response_filename)
  args = shlex.split(args)
  return args
