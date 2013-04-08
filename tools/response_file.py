import tempfile, os, sys, shlex
from tempfiles import try_delete

# Routes the given cmdline param list in args into a new response file and returns the filename to it.
# The response file has a suffix '.tmp' to signal that the process receiving the response file is free to delete it after it has consumed it.
# The returned filename has '@' prepended to it already for convenience.
def create_response_file(args, directory):
  (response_fd, response_filename) = tempfile.mkstemp(prefix='emscripten_', suffix='.tmp', dir=directory, text=True)
  response_fd = os.fdopen(response_fd, "w")
  #print >> sys.stderr, "Creating response file '%s'" % response_filename
  args = map(lambda p: p.replace(' ', '').replace('\\', '\\\\').replace('"', '\\"'), args)
  response_fd.write(' '.join(args))
  response_fd.close()
  return '@' + response_filename

# Reads a response file, and returns the list of cmdline params found in the file.
# If the response file ends with .tmp, it is automatically deleted after reading it.
def read_and_delete_response_file(response_filename):
  if response_filename.startswith('@'):
    response_filename = response_filename[1:]

  #print >> sys.stderr, "Using response file '%s'" % response_filename
  if not os.path.exists(response_filename):
    raise Exception("Response file '%s' not found!" % response_filename)

  response_fd = open(response_filename, 'r')
  args = response_fd.read()
  response_fd.close()
  # For conveniency, the receiver is allowed to immediately clean up response files ending with '.tmp' so that the
  # caller doesn't have to do it.
  if response_filename.endswith(".tmp"):
    try_delete(response_filename)
  args = shlex.split(args)
  return args
