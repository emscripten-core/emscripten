#!/usr/bin/python

# This is a helper script to validate a file for asm.js.

# cmdline usage: 'python validate_asmjs.py filename.{html/js}'
# Prints a line starting with 'OK: ' on success, and returns process exit code 0.
# On failure, prints a line starting with 'FAIL: ', and returns a nonzero process exit code.

# python usage: 'validate_asmjs("filename.{html/js}", muteOutput=True/False)'
# Returns True/False depending on whether the file was valid asm.js.

# This script depends on the SpiderMonkey JS engine, which must be present in PATH in order for this script to function.

import subprocess, sys, re, tempfile, os, time
import shared

# Looks up SpiderMonkey engine using the variable SPIDERMONKEY_ENGINE in ~/.emscripten, and if not set up there, via PATH.
def find_spidermonkey_engine():
  sm_engine = shared.SPIDERMONKEY_ENGINE if hasattr(shared, 'SPIDERMONKEY_ENGINE') else ['']
  if not sm_engine or len(sm_engine[0]) == 0 or not os.path.exists(sm_engine[0]):
    sm_engine[0] = shared.Building.which('js')
    if sm_engine[0] == None:
      return ['js-not-found']
  return sm_engine

# Given a .js file, returns True/False depending on if that file is valid asm.js
def validate_asmjs_jsfile(filename, muteOutput):
  process = subprocess.Popen(find_spidermonkey_engine() + ['-c', filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
  (stdout, stderr) = process.communicate()
  if not muteOutput:
    if len(stdout.strip()) > 0:
      print stdout.strip()
    if len(stderr.strip()) > 0:
      # Pretty-print the output not to contain a spurious warning.
      stderr = stderr.replace('warning: successfully compiled asm.js', ' successfully compiled asm.js')

      print >> sys.stderr, stderr.strip()
  if 'successfully compiled asm.js' in stderr:
    return True
  else:
    return False

# This tool takes as input a file built with Emscripten (either .html or .js) and validates it for asm.js.
# Returns True/False denoting whether the file was valid asm.js. In case of a .html file, all <script>content</script> tags are searched,
# and the ones containing a "use asm" section are validated.
def validate_asmjs(filename, muteOutput):
  if filename.endswith('.html'):
    html = open(filename, 'r').read()
    matches = re.findall('''<\w*script\w*.*?>(.*?)<\w*/script\w*>''', html, re.DOTALL | re.MULTILINE)
    numAsmJsBlocks = 0
    for match in matches:
      if '"use asm"' in match:
        numAsmJsBlocks = numAsmJsBlocks + 1
        tmp_js = tempfile.mkstemp(suffix='.js')
        os.write(tmp_js[0], match)
        os.close(tmp_js[0])
        valid_asmjs = validate_asmjs_jsfile(tmp_js[1], muteOutput)
        os.remove(tmp_js[1])
        if not valid_asmjs:
          return False
    if numAsmJsBlocks == 0:
      if not muteOutput:
        print >> sys.stderr, 'Error: the file does not contain any "use asm" modules.'
      return False
    else:
      return True
  else:
    return validate_asmjs_jsfile(filename, muteOutput)

def main():
  if len(sys.argv) < 2:
    print 'Usage: validate_asmjs <filename>'
    return 2
  if validate_asmjs(sys.argv[1], muteOutput=False):
    print "OK: File '" + sys.argv[1] + "' validates as asm.js"
    return 0
  else:
    print "FAIL: File '" + sys.argv[1] + "' is not valid asm.js"
    return 1

if __name__ == '__main__':
  sys.exit(main())
