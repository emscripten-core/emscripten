import sys

# This function checks and prints out the detected line endings in the given file.
# If the file only contains either Windows \r\n line endings or Unix \n line endings, it returns 0.
# Otherwise, in the presence of old OSX or mixed/malformed line endings, a non-zero error code is returned.
def check_line_endings(filename, print_errors=True):
  try:
    data = open(filename, 'rb').read()
  except Exception, e:
    if print_errors: print >> sys.stderr, "Unable to read file '" + filename + "'! " + str(e)
    return 1
  if len(data) == 0:
    if print_errors: print >> sys.stderr, "Unable to read file '" + filename + "', or file was empty!"
    return 1

  if "\r\r\n" in data:
    if print_errors: print >> sys.stderr, "File '" + filename + "' contains BAD line endings of form \\r\\r\\n!"
    return 1 # Bad line endings in file, return a non-zero process exit code.

  has_dos_line_endings = False
  has_unix_line_endings = False
  if '\r\n' in data:
    has_dos_line_endings = True
    data = data.replace('\r\n', 'A') # Replace all DOS line endings with some other character, and continue testing what's left.
  if '\n' in data:
    has_unix_line_endings = True
  if '\r' in data:
    if print_errors: print >> sys.stderr, 'File \'' + filename + '\' contains OLD OSX line endings "\\r"'
    return 1 # Return a non-zero process exit code since we don't want to use the old OSX (9.x) line endings anywhere.
  if has_dos_line_endings and has_unix_line_endings:
    if print_errors: print >> sys.stderr, 'File \'' + filename + '\' contains both DOS "\\r\\n" and UNIX "\\n" line endings!'
    return 1 # Mixed line endings
  else: return 0

if __name__ == '__main__':
  if len(sys.argv) != 2:
    print >> sys.stderr, 'Unknown command line ' + str(sys.argv) + '!'
    print >> sys.stderr, 'Usage: ' + sys.argv[0] + ' <filename>'
    sys.exit(1)
  sys.exit(check_line_endings(sys.argv[1]))
