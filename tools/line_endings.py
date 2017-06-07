import sys, shutil, os

def convert_line_endings(text, from_eol, to_eol):
  if from_eol == to_eol: return text
  return text.replace(from_eol, to_eol)

# if dst_filename is empty, operates in place on src_filename
def convert_line_endings_in_file(filename, from_eol, to_eol):
  if from_eol == to_eol: return # No conversion needed

  text = open(filename, 'rb').read()
  text = convert_line_endings(text, from_eol, to_eol)
  open(filename, 'wb').write(text)

# This function checks and prints out the detected line endings in the given file.
# If the file only contains either Windows \r\n line endings or Unix \n line endings, it returns 0.
# Otherwise, in the presence of old OSX or mixed/malformed line endings, a non-zero error code is returned.
def check_line_endings(filename, expect_only_specific_line_endings=None, print_errors=True, print_info=False):
  try:
    data = open(filename, 'rb').read()
  except Exception, e:
    if print_errors: print >> sys.stderr, "Unable to read file '" + filename + "'! " + str(e)
    return 1
  if len(data) == 0:
    if print_errors: print >> sys.stderr, "Unable to read file '" + filename + "', or file was empty!"
    return 1

  bad_line_ending_index = data.find("\r\r\n")
  if bad_line_ending_index != -1:
    if print_errors:
      print >> sys.stderr, "File '" + filename + "' contains BAD line endings of form \\r\\r\\n!"
      bad_line = data[max(0,bad_line_ending_index-50):min(len(data), bad_line_ending_index+50)]
      bad_line = bad_line.replace('\r', '\\r').replace('\n', '\\n')
      print >> sys.stderr, "Content around the location: '" + bad_line + "'"
    return 1 # Bad line endings in file, return a non-zero process exit code.

  has_dos_line_endings = False
  has_unix_line_endings = False
  dos_line_ending_example = ''
  dos_line_ending_count = 0
  unix_line_ending_example = ''
  unix_line_ending_count = 0
  if '\r\n' in data:
    dos_line_ending_example = data[max(0, data.find('\r\n') - 50):min(len(data), data.find('\r\n')+50)].replace('\r', '\\r').replace('\n', '\\n')
    dos_line_ending_count = data.count('\r\n')
    has_dos_line_endings = True
    data = data.replace('\r\n', 'A') # Replace all DOS line endings with some other character, and continue testing what's left.
  if '\n' in data:
    unix_line_ending_example = data[max(0, data.find('\n') - 50):min(len(data), data.find('\n')+50)].replace('\r', '\\r').replace('\n', '\\n')
    unix_line_ending_count = data.count('\n')
    has_unix_line_endings = True
  if '\r' in data:
    old_osx_line_ending_example = data[max(0, data.find('\r') - 50):min(len(data), data.find('\r')+50)].replace('\r', '\\r').replace('\n', '\\n')
    if print_errors:
      print >> sys.stderr, 'File \'' + filename + '\' contains OLD OSX line endings "\\r"'
      print >> sys.stderr, "Content around an OLD OSX line ending location: '" + old_osx_line_ending_example + "'"
    return 1 # Return a non-zero process exit code since we don't want to use the old OSX (9.x) line endings anywhere.
  if has_dos_line_endings and has_unix_line_endings:
    if print_errors:
      print >> sys.stderr, 'File \'' + filename + '\' contains both DOS "\\r\\n" and UNIX "\\n" line endings! (' + str(dos_line_ending_count) + ' DOS line endings, ' + str(unix_line_ending_count) + ' UNIX line endings)'
      print >> sys.stderr, "Content around a DOS line ending location: '" + dos_line_ending_example + "'"
      print >> sys.stderr, "Content around an UNIX line ending location: '" + unix_line_ending_example + "'"
    return 1 # Mixed line endings
  elif print_info:
    if has_dos_line_endings: print 'File \'' + filename + '\' contains DOS "\\r\\n" line endings.'
    if has_unix_line_endings: print 'File \'' + filename +'\' contains UNIX "\\n" line endings.'
  if expect_only_specific_line_endings == '\n' and has_dos_line_endings:
    if print_errors:
      print >> sys.stderr, 'File \'' + filename + '\' contains DOS "\\r\\n" line endings! (' + str(dos_line_ending_count) + ' DOS line endings), but expected only UNIX line endings!'
      print >> sys.stderr, "Content around a DOS line ending location: '" + dos_line_ending_example + "'"
    return 1 # DOS line endings, but expected UNIX
  if expect_only_specific_line_endings == '\r\n' and has_unix_line_endings:
    if print_errors:
      print >> sys.stderr, 'File \'' + filename + '\' contains UNIX "\\n" line endings! (' + str(unix_line_ending_count) + ' UNIX line endings), but expected only DOS line endings!'
      print >> sys.stderr, "Content around a UNIX line ending location: '" + unix_line_ending_example + "'"
    return 1 # UNIX line endings, but expected DOS
  else: return 0

if __name__ == '__main__':
  if len(sys.argv) != 2:
    print >> sys.stderr, 'Unknown command line ' + str(sys.argv) + '!'
    print >> sys.stderr, 'Usage: ' + sys.argv[0] + ' <filename>'
    sys.exit(1)
  sys.exit(check_line_endings(sys.argv[1], print_info=True))
