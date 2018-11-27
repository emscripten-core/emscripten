#!/usr/bin/python
# Copyright 2013 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

# This is a helper script to validate a file for asm.js.

# cmdline usage: 'python validate_asmjs.py filename.{html/js}'
# Prints a line starting with 'OK: ' on success, and returns process exit code 0.
# On failure, prints a line starting with 'FAIL: ', and returns a nonzero process exit code.

# python usage: 'validate_asmjs("filename.{html/js}", muteOutput=True/False)'
# Returns True/False depending on whether the file was valid asm.js.

# This script depends on the SpiderMonkey JS engine, which must be present in PATH in order for this script to function.

from __future__ import print_function
import subprocess, sys, re, tempfile, os, time

sys.path.insert(1, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tools import shared

# Given a .js file, returns True/False depending on if that file is valid asm.js
def validate_asmjs_jsfile(filename, muteOutput):
  cmd = shared.SPIDERMONKEY_ENGINE + ['-c', filename]
  if not shared.SPIDERMONKEY_ENGINE or cmd[0] == 'js-not-found' or len(cmd[0].strip()) == 0:
    print('Could not find SpiderMonkey engine! Please set its location to SPIDERMONKEY_ENGINE in your ' + shared.hint_config_file_location() + ' configuration file!', file=sys.stderr)
    return False
  try:
    process = shared.run_process(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=subprocess.PIPE)
  except Exception as e:
    print('Executing command ' + str(cmd) + ' failed due to an exception: ' + str(e) + '!', file=sys.stderr)
    return False
  stdout = process.stdout
  stderr = process.stderr
  if not muteOutput:
    if len(stdout.strip()):
      print(stdout.strip())
    if len(stderr.strip()):
      # Pretty-print the output not to contain a spurious warning.
      warning_re = re.compile(re.escape('warning: successfully compiled asm.js'), re.IGNORECASE)
      stderr = warning_re.sub(' successfully compiled asm.js', stderr)
      print(stderr.strip(), file=sys.stderr)
  if 'successfully compiled asm.js' in stderr.lower():
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
      # Test a .js file with the same basename - emcc convention
      # is to generate files with same basename but different suffix.
      js_file = filename.replace('.html', '.js')
      if os.path.isfile(js_file):
        return validate_asmjs(js_file, muteOutput)
      if not muteOutput:
        print('Error: the file does not contain any "use asm" modules.', file=sys.stderr)
      return False
    else:
      return True
  else:
    return validate_asmjs_jsfile(filename, muteOutput)

def main():
  if len(sys.argv) < 2:
    print('Usage: validate_asmjs <filename>')
    return 2
  if validate_asmjs(sys.argv[1], muteOutput=False):
    print("OK: File '" + sys.argv[1] + "' validates as asm.js")
    return 0
  else:
    print("FAIL: File '" + sys.argv[1] + "' is not valid asm.js")
    return 1

if __name__ == '__main__':
  sys.exit(main())
