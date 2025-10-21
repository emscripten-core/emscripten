#!/usr/bin/env python3
# Copyright 2012 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

"""A tool that generates FS API calls to generate a filesystem, and packages the files
to work with that.

This is called by emcc. You can also call it yourself.

You can split your files into "asset bundles", and create each bundle separately
with this tool. Then just include the generated js for each and they will load
the data and prepare it accordingly. This allows you to share assets and reduce
data downloads.

 * If you run this yourself, separately/standalone from emcc, then the main program
   compiled by emcc must be built with filesystem support. You can do that with
   -sFORCE_FILESYSTEM (if you forget that, an unoptimized build or one with
   ASSERTIONS enabled will show an error suggesting you use that flag).

Usage:

  file_packager TARGET [--preload A [B..]] [--embed C [D..]] [--exclude E [F..]] [--js-output=OUTPUT.js] [--no-force] [--use-preload-cache] [--indexedDB-name=EM_PRELOAD_CACHE] [--separate-metadata] [--lz4] [--use-preload-plugins] [--no-node] [--export-es6] [--help]

  --preload  ,
  --embed    See emcc --help for more details on those options.

  --exclude E [F..] Specifies filename pattern matches to use for excluding given files from being added to the package.
                    See https://docs.python.org/2/library/fnmatch.html for syntax.

  --from-emcc Indicate that `file_packager` was called from `emcc` and will be further processed by it, so some code generation can be skipped here

  --js-output=FILE Writes output in FILE, if not specified, standard output is used.

  --obj-output=FILE create an object file from embedded files, for direct linking into a wasm binary.

  --depfile=FILE Writes a dependency list containing the list of directories and files walked, compatible with Make, Ninja, CMake, etc.

  --wasm64 When used with `--obj-output` create a wasm64 object file

  --export-name=EXPORT_NAME Use custom export name (default is `Module`)

  --export-es6 Wrap generated code inside ES6 exported function

  --no-force Don't create output if no valid input file is specified.

  --use-preload-cache Stores package in IndexedDB so that subsequent loads don't need to do XHR. Checks package version.

  --indexedDB-name Use specified IndexedDB database name (Default: 'EM_PRELOAD_CACHE')

  --separate-metadata Stores package metadata separately. Only applicable when preloading and js-output file is specified.

  --lz4 Uses LZ4. This compresses the data using LZ4 when this utility is run, then the client decompresses chunks on the fly, avoiding storing
        the entire decompressed data in memory at once. See LZ4 in src/settings.js, you must build the main program with that flag.

  --use-preload-plugins Tells the file packager to run preload plugins on the files as they are loaded. This performs tasks like decoding images
                        and audio using the browser's codecs.

  --no-node Whether to support Node.js. By default we do, which emits some extra code.

  --quiet Suppress reminder about using `FORCE_FILESYSTEM`

Notes:

  * The file packager generates unix-style file paths. So if you are on windows and a file is accessed at
    subdir\file, in JS it will be subdir/file. For simplicity we treat the web platform as a *NIX.
"""

import base64
import ctypes
import fnmatch
import hashlib
import json
import os
import posixpath
import shutil
import sys
from dataclasses import dataclass
from subprocess import PIPE
from textwrap import dedent
from typing import List

__scriptdir__ = os.path.dirname(os.path.abspath(__file__))
__rootdir__ = os.path.dirname(__scriptdir__)
sys.path.insert(0, __rootdir__)

from tools import diagnostics, js_manipulation, shared, utils
from tools.response_file import substitute_response_files

DEBUG = os.environ.get('EMCC_DEBUG')

excluded_patterns: List[str] = []
new_data_files = []
walked = []


class Options:
  def __init__(self):
    self.export_name = 'Module'
    self.has_preloaded = False
    self.has_embedded = False
    self.jsoutput = None
    self.obj_output = None
    self.depfile = None
    self.from_emcc = False
    self.quiet = False
    self.force = True
    # If set to True, IndexedDB (IDBFS in library_idbfs.js) is used to locally
    # cache VFS XHR so that subsequent page loads can read the data from the
    # offline cache instead.
    self.use_preload_cache = False
    self.indexeddb_name = 'EM_PRELOAD_CACHE'
    # If set to True, the package metadata is stored separately from js-output
    # file which makes js-output file immutable to the package content changes.
    # If set to False, the package metadata is stored inside the js-output file
    # which makes js-output file to mutate on each invocation of this packager tool.
    self.separate_metadata = False
    self.lz4 = False
    self.use_preload_plugins = False
    self.support_node = True
    self.wasm64 = False
    self.export_es6 = False


@dataclass
class DataFile:
  srcpath: str
  dstpath: str
  mode: str
  explicit_dst_path: bool


options = Options()


def err(*args):
  print(*args, file=sys.stderr)


def base64_encode(b):
  b64 = base64.b64encode(b)
  return b64.decode('ascii')


def has_hidden_attribute(filepath):
  """Win32 code to test whether the given file has the hidden property set."""

  if sys.platform != 'win32':
    return False

  try:
    attrs = ctypes.windll.kernel32.GetFileAttributesW(filepath)
    assert attrs != -1
    result = bool(attrs & 2)
  except Exception:
    result = False
  return result


def should_ignore(fullname):
  """The packager should never preload/embed files if the file
  is hidden (Win32) or it matches any pattern specified in --exclude"""
  if has_hidden_attribute(fullname):
    return True

  return any(fnmatch.fnmatch(fullname, p) for p in excluded_patterns)


def add(mode, rootpathsrc, rootpathdst):
  """Expand directories into individual files

  rootpathsrc: The path name of the root directory on the local FS we are
               adding to emscripten virtual FS.
  rootpathdst: The name we want to make the source path available on the
               emscripten virtual FS.
  """
  walked.append(rootpathsrc)
  for dirpath, dirnames, filenames in os.walk(rootpathsrc):
    new_dirnames = []
    for name in dirnames:
      fullname = os.path.join(dirpath, name)
      if not should_ignore(fullname):
        walked.append(fullname)
        new_dirnames.append(name)
      elif DEBUG:
        err('Skipping directory "%s" from inclusion in the emscripten '
            'virtual file system.' % fullname)
    for name in filenames:
      fullname = os.path.join(dirpath, name)
      if not should_ignore(fullname):
        walked.append(fullname)
        # Convert source filename relative to root directory of target FS.
        dstpath = os.path.join(rootpathdst,
                               os.path.relpath(fullname, rootpathsrc))
        new_data_files.append(DataFile(srcpath=fullname, dstpath=dstpath,
                                       mode=mode, explicit_dst_path=True))
      elif DEBUG:
        err('Skipping file "%s" from inclusion in the emscripten '
            'virtual file system.' % fullname)
    dirnames.clear()
    dirnames.extend(new_dirnames)


def to_asm_string(string):
  """Convert a python string to string suitable for including in an
  assembly file using the `.asciz` directive.

  The result will be an UTF-8 encoded string in the data section.
  """
  # See MCAsmStreamer::PrintQuotedString in llvm/lib/MC/MCAsmStreamer.cpp
  # And isPrint in llvm/include/llvm/ADT/StringExtras.h

  def is_print(c):
    return c >= 0x20 and c <= 0x7E

  def escape(c):
    if is_print(c):
      return chr(c)
    escape_chars = {
      '\b': '\\b',
      '\f': '\\f',
      '\n': '\\n',
      '\r': '\\r',
      '\t': '\\t',
    }
    if c in escape_chars:
      return escape_chars[c]
    # Enscode all other chars are three octal digits(!)
    return '\\%s%s%s' % (oct(c >> 6), oct(c >> 3), oct(c >> 0))

  return ''.join(escape(c) for c in string.encode('utf-8'))


def to_c_symbol(filename, used):
  """Convert a filename (python string) to a legal C symbols, avoiding collisions."""
  def escape(c):
     if c.isalnum():
       return c
     else:
       return '_'
  c_symbol = ''.join(escape(c) for c in filename)
  # Handle collisions
  if c_symbol in used:
    counter = 2
    while c_symbol + str(counter) in used:
      counter = counter + 1
    c_symbol = c_symbol + str(counter)
  used.add(c_symbol)
  return c_symbol


def generate_object_file(data_files):
  embed_files = [f for f in data_files if f.mode == 'embed']
  assert embed_files

  asm_file = utils.replace_suffix(options.obj_output, '.s')

  used = set()
  for f in embed_files:
    f.c_symbol_name = '__em_file_data_%s' % to_c_symbol(f.dstpath, used)

  with open(asm_file, 'w') as out:
    out.write('# Emscripten embedded file data, generated by tools/file_packager.py\n')

    for f in embed_files:
      if DEBUG:
        err('embedding %s at %s' % (f.srcpath, f.dstpath))

      size = os.path.getsize(f.srcpath)
      dstpath = to_asm_string(f.dstpath)
      srcpath = utils.normalize_path(f.srcpath)
      out.write(dedent(f'''
      .section .rodata.{f.c_symbol_name},"",@

      # The name of file
      {f.c_symbol_name}_name:
      .asciz "{dstpath}"
      .size {f.c_symbol_name}_name, {len(dstpath) + 1}

      # The size of the file followed by the content itself
      {f.c_symbol_name}:
      .incbin "{srcpath}"
      .size {f.c_symbol_name}, {size}
      '''))

    if options.wasm64:
      align = 3
      ptr_type = 'i64'
      bits = 64
    else:
      align = 2
      ptr_type = 'i32'
      bits = 32
    out.write(dedent(f'''
      .functype _emscripten_fs_load_embedded_files ({ptr_type}) -> ()
      .section .text,"",@
      init_file_data:
        .functype init_file_data () -> ()
        global.get __emscripten_embedded_file_data@GOT
        call _emscripten_fs_load_embedded_files
        end_function

      # Run init_file_data on startup.
      # See system/lib/README.md for ordering of system constructors.
      .section .init_array.49,"",@
      .p2align {align}
      .int{bits} init_file_data

      # A list of triples of:
      # (file_name_ptr, file_data_size, file_data_ptr)
      # The list in null terminate with a single 0
      .section .rodata.__emscripten_embedded_file_data,"",@
      __emscripten_embedded_file_data:
      .p2align {align}
      '''))

    for f in embed_files:
      # The `.dc.a` directive gives us a pointer (address) sized entry.
      # See https://sourceware.org/binutils/docs/as/Dc.html
      out.write(dedent(f'''\
        .p2align %s
        .dc.a {f.c_symbol_name}_name
        .p2align %s
        .int32 {os.path.getsize(f.srcpath)}
        .p2align %s
        .dc.a {f.c_symbol_name}
        ''' % (align, align, align)))

    ptr_size = 4
    elem_size = (2 * ptr_size) + 4
    total_size = len(embed_files) * elem_size + 4
    out.write(dedent(f'''\
      .dc.a 0
      .size __emscripten_embedded_file_data, {total_size}
      '''))
  cmd = [shared.EMCC, '-c', asm_file, '-o', options.obj_output]
  if options.wasm64:
    target = 'wasm64-unknown-emscripten'
    cmd.append('-Wno-experimental')
  else:
    target = 'wasm32-unknown-emscripten'
  cmd.append('--target=' + target)
  shared.check_call(cmd)


def main():  # noqa: C901, PLR0912, PLR0915
  """Future modifications should consider refactoring to reduce complexity.

  * The McCabe cyclomatiic complexity is currently 60 vs 10 recommended.
  * There are currently 63 branches vs 12 recommended.
  * There are currently 151 statements vs 50 recommended.

  To revalidate these numbers, run `ruff check --select=C901,PLR091`.
  """
  if len(sys.argv) == 1:
    err('''Usage: file_packager TARGET [--preload A [B..]] [--embed C [D..]] [--exclude E [F..]] [--js-output=OUTPUT.js] [--no-force] [--use-preload-cache] [--indexedDB-name=EM_PRELOAD_CACHE] [--separate-metadata] [--lz4] [--use-preload-plugins] [--no-node] [--export-es6] [--help]
  Try 'file_packager --help' for more details.''')
    return 1

  # read response files very early on
  try:
    args = substitute_response_files(sys.argv[1:])
  except OSError as e:
    shared.exit_with_error(e)

  if '--help' in args:
    print(__doc__.strip())
    return 0

  data_target = args[0]
  data_files = []
  leading = ''

  for arg in args[1:]:
    if arg == '--preload':
      leading = 'preload'
    elif arg == '--embed':
      leading = 'embed'
    elif arg == '--exclude':
      leading = 'exclude'
    elif arg == '--no-force':
      options.force = False
      leading = ''
    elif arg == '--export-es6':
      options.export_es6 = True
      leading = ''
    elif arg == '--use-preload-cache':
      options.use_preload_cache = True
      leading = ''
    elif arg.startswith('--indexedDB-name'):
      options.indexeddb_name = arg.split('=', 1)[1] if '=' in arg else None
      leading = ''
    elif arg == '--no-heap-copy':
      diagnostics.warn('ignoring legacy flag --no-heap-copy (that is the only mode supported now)')
      leading = ''
    elif arg == '--separate-metadata':
      options.separate_metadata = True
      leading = ''
    elif arg == '--lz4':
      options.lz4 = True
      leading = ''
    elif arg == '--use-preload-plugins':
      options.use_preload_plugins = True
      leading = ''
    elif arg == '--no-node':
      options.support_node = False
      leading = ''
    elif arg.startswith('--js-output'):
      options.jsoutput = arg.split('=', 1)[1] if '=' in arg else None
      leading = ''
    elif arg.startswith('--obj-output'):
      options.obj_output = arg.split('=', 1)[1] if '=' in arg else None
      leading = ''
    elif arg.startswith('--depfile'):
      options.depfile = arg.split('=', 1)[1] if '=' in arg else None
      leading = ''
    elif arg == '--wasm64':
      options.wasm64 = True
    elif arg.startswith('--export-name'):
      if '=' in arg:
        options.export_name = arg.split('=', 1)[1]
      leading = ''
    elif arg == '--from-emcc':
      options.from_emcc = True
      leading = ''
    elif arg == '--quiet':
      options.quiet = True
    elif leading in {'preload', 'embed'}:
      mode = leading
      # position of @ if we're doing 'src@dst'. '__' is used to keep the index
      # same with the original if they escaped with '@@'.
      at_position = arg.replace('@@', '__').find('@')
      # '@@' in input string means there is an actual @ character, a single '@'
      # means the 'src@dst' notation.
      uses_at_notation = (at_position != -1)

      if uses_at_notation:
        srcpath = arg[0:at_position].replace('@@', '@') # split around the @
        dstpath = arg[at_position + 1:].replace('@@', '@')
      else:
        # Use source path as destination path.
        srcpath = dstpath = arg.replace('@@', '@')
      if os.path.isfile(srcpath) or os.path.isdir(srcpath):
        data_files.append(DataFile(srcpath=srcpath, dstpath=dstpath, mode=mode,
                                   explicit_dst_path=uses_at_notation))
      else:
        diagnostics.error(f'${arg} does not exist')
    elif leading == 'exclude':
      excluded_patterns.append(arg)
    else:
      diagnostics.error('Unknown parameter:', arg)

  options.has_preloaded = any(f.mode == 'preload' for f in data_files)
  options.has_embedded = any(f.mode == 'embed' for f in data_files)

  if options.has_preloaded and options.has_embedded:
    diagnostics.warn('support for using --preload and --embed in the same command is scheduled '
        'for deprecation.  If you need this feature please comment at '
        'https://github.com/emscripten-core/emscripten/issues/24803')

  if options.separate_metadata and (not options.has_preloaded or not options.jsoutput):
    diagnostics.error('cannot separate-metadata without both --preloaded files and a specified --js-output')

  if not options.from_emcc and not options.quiet:
    diagnostics.warn('Remember to build the main file with `-sFORCE_FILESYSTEM` '
        'so that it includes support for loading this file package')

  if options.jsoutput and os.path.abspath(options.jsoutput) == os.path.abspath(data_target):
    diagnostics.error('TARGET should not be the same value of --js-output')

  if options.from_emcc and options.export_es6:
    diagnostics.error("Can't use --export-es6 option together with --from-emcc since the code should be embedded within emcc's code")

  walked.append(__file__)
  for file_ in data_files:
    if not should_ignore(file_.srcpath):
      if os.path.isdir(file_.srcpath):
        add(file_.mode, file_.srcpath, file_.dstpath)
      else:
        walked.append(file_.srcpath)
        new_data_files.append(file_)
  data_files = [file_ for file_ in new_data_files
                if not os.path.isdir(file_.srcpath)]
  if len(data_files) == 0:
    diagnostics.error('Nothing to do!')

  # Absolutize paths, and check that they make sense
  # os.getcwd() always returns the hard path with any symbolic links resolved,
  # even if we cd'd into a symbolic link.
  curr_abspath = os.path.abspath(os.getcwd())

  for file_ in data_files:
    if not file_.explicit_dst_path:
      # This file was not defined with src@dst, so we inferred the destination
      # from the source. In that case, we require that the destination be
      # within the current working directory.
      path = file_.dstpath
      # Use os.path.realpath to resolve any symbolic links to hard paths,
      # to match the structure in curr_abspath.
      abspath = os.path.realpath(os.path.abspath(path))
      if DEBUG:
        err(path, abspath, curr_abspath)
      if not abspath.startswith(curr_abspath):
        err('Error: Embedding "%s" which is not contained within the current directory '
            '"%s".  This is invalid since the current directory becomes the '
            'root that the generated code will see.  To include files outside of the current '
            'working directory you can use the `--preload-file srcpath@dstpath` syntax to '
            'explicitly specify the target location.' % (path, curr_abspath))
        sys.exit(1)
      file_.dstpath = abspath[len(curr_abspath) + 1:]
      if os.path.isabs(path):
        diagnostics.warn('Embedding an absolute file/directory name "%s" to the '
            'virtual filesystem. The file will be made available in the '
            'relative path "%s". You can use the `--preload-file srcpath@dstpath` '
            'syntax to explicitly specify the target location the absolute source '
            'path should be directed to.' % (path, file_.dstpath))

  for file_ in data_files:
    # name in the filesystem, native and emulated
    file_.dstpath = utils.normalize_path(file_.dstpath)
    # If user has submitted a directory name as the destination but omitted
    # the destination filename, use the filename from source file
    if file_.dstpath.endswith('/'):
      file_.dstpath = file_.dstpath + os.path.basename(file_.srcpath)
    # make destination path always relative to the root
    file_.dstpath = posixpath.normpath(os.path.join('/', file_.dstpath))
    if DEBUG:
      err('Packaging file "%s" to VFS in path "%s".' % (file_.srcpath,  file_.dstpath))

  # Remove duplicates (can occur naively, for example preload dir/, preload dir/subdir/)
  seen = set()

  def was_seen(name):
    if name in seen:
      return True
    seen.add(name)
    return False

  # The files are sorted by the dstpath to make the order of files reproducible
  # across file systems / operating systems (os.walk does not produce the same
  # file order on different file systems / operating systems)
  data_files = sorted(data_files, key=lambda file_: file_.dstpath)
  data_files = [file_ for file_ in data_files if not was_seen(file_.dstpath)]

  metadata = {'files': []}

  if options.depfile:
    targets = []
    if options.obj_output:
      targets.append(options.obj_output)
    if options.jsoutput:
      targets.append(data_target)
      targets.append(options.jsoutput)
    with open(options.depfile, 'w') as f:
      for target in targets:
        if target:
          f.write(escape_for_makefile(target))
          f.write(' \\\n')
      f.write(': \\\n')
      for dependency in walked:
        f.write(escape_for_makefile(dependency))
        f.write(' \\\n')

  if options.obj_output:
    if not options.has_embedded:
      diagnostics.error('--obj-output is only applicable when embedding files')
    generate_object_file(data_files)
    if not options.has_preloaded:
      return 0

  ret = generate_js(data_target, data_files, metadata)

  if options.force or data_files:
    if options.jsoutput is None:
      print(ret)
    else:
      # Overwrite the old jsoutput file (if exists) only when its content
      # differs from the current generated one, otherwise leave the file
      # untouched preserving its old timestamp
      if os.path.isfile(options.jsoutput):
        old = utils.read_file(options.jsoutput)
        if old != ret:
          utils.write_file(options.jsoutput, ret)
      else:
        utils.write_file(options.jsoutput, ret)
      if options.separate_metadata:
        utils.write_file(options.jsoutput + '.metadata', json.dumps(metadata, separators=(',', ':')))

  return 0


def escape_for_makefile(fpath):
  # Escapes for CMake's "pathname" grammar as described here:
  #   https://cmake.org/cmake/help/latest/command/add_custom_command.html#grammar-token-depfile-pathname
  # Which is congruent with how Ninja and GNU Make expect characters escaped.
  fpath = utils.normalize_path(fpath)
  return fpath.replace('$', '$$').replace('#', '\\#').replace(' ', '\\ ')


def generate_js(data_target, data_files, metadata):
  # emcc will add this to the output itself, so it is only needed for
  # standalone calls
  if options.from_emcc:
    ret = ''
  else:
    if options.export_es6:
      ret = 'export default async function loadDataFile(Module) {\n'
    else:
      ret = '''
  var Module = typeof %(EXPORT_NAME)s != 'undefined' ? %(EXPORT_NAME)s : {};\n''' % {"EXPORT_NAME": options.export_name}

  ret += '''
  if (!Module['expectedDataFileDownloads']) Module['expectedDataFileDownloads'] = 0;
  Module['expectedDataFileDownloads']++;'''

  if not options.export_es6:
    ret += '''
  (() => {'''

  ret += '''
    // Do not attempt to redownload the virtual filesystem data when in a pthread or a Wasm Worker context.
    var isPthread = typeof ENVIRONMENT_IS_PTHREAD != 'undefined' && ENVIRONMENT_IS_PTHREAD;
    var isWasmWorker = typeof ENVIRONMENT_IS_WASM_WORKER != 'undefined' && ENVIRONMENT_IS_WASM_WORKER;
    if (isPthread || isWasmWorker) return;\n'''

  if options.support_node:
    ret += "    var isNode = globalThis.process && globalThis.process.versions && globalThis.process.versions.node && globalThis.process.type != 'renderer';\n"

  if options.support_node and options.export_es6:
        ret += '''if (isNode) {
    const { createRequire } = await import('module');
    /** @suppress{duplicate} */
    var require = createRequire(import.meta.url);
  }\n'''

  if options.export_es6:
    ret += 'return new Promise((loadDataResolve, loadDataReject) => {\n'
  ret += '    async function loadPackage(metadata) {\n'

  code = '''
      function assert(check, msg) {
        if (!check) throw new Error(msg);
      }\n'''

  # Set up folders
  partial_dirs = []
  for file_ in data_files:
    dirname = os.path.dirname(file_.dstpath)
    dirname = dirname.lstrip('/') # absolute paths start with '/', remove that
    if dirname != '':
      parts = dirname.split('/')
      for i in range(len(parts)):
        partial = '/'.join(parts[:i + 1])
        if partial not in partial_dirs:
          code += ('''Module['FS_createPath'](%s, %s, true, true);\n'''
                   % (json.dumps('/' + '/'.join(parts[:i])), json.dumps(parts[i])))
          partial_dirs.append(partial)

  if options.has_preloaded:
    # Bundle all datafiles into one archive. Avoids doing lots of simultaneous
    # XHRs which has overhead.
    start = 0
    with open(data_target, 'wb') as data:
      for file_ in data_files:
        file_.data_start = start
        curr = utils.read_binary(file_.srcpath)
        file_.data_end = start + len(curr)
        start += len(curr)
        data.write(curr)

    if start > 256 * 1024 * 1024:
      diagnostics.warn('file packager is creating an asset bundle of %d MB. '
          'this is very large, and browsers might have trouble loading it. '
          'see https://hacks.mozilla.org/2015/02/synchronous-execution-and-filesystem-access-in-emscripten/'
          % (start / (1024 * 1024)))

    create_preloaded = '''
          try {
            // canOwn this data in the filesystem, it is a slice into the heap that will never change
            await Module['FS_preloadFile'](name, null, data, true, true, false, true);
            Module['removeRunDependency'](`fp ${name}`);
          } catch (e) {
            err(`Preloading file ${name} failed`, e);
          }\n'''
    create_data = '''// canOwn this data in the filesystem, it is a slice into the heap that will never change
          Module['FS_createDataFile'](name, null, data, true, true, true);
          Module['removeRunDependency'](`fp ${name}`);'''

    finish_handler = create_preloaded if options.use_preload_plugins else create_data

    if not options.lz4:
      # Data requests - for getting a block of data out of the big archive - have
      # a similar API to XHRs
      code += '''
      for (var file of metadata['files']) {
        var name = file['filename']
        Module['addRunDependency'](`fp ${name}`);
      }\n'''

  if options.has_embedded and not options.obj_output:
    diagnostics.warn('--obj-output is recommended when using --embed.  This outputs an object file for linking directly into your application is more efficient than JS encoding')

  catch_handler = ''
  if options.export_es6:
    catch_handler += '''
        .catch((error) => {
          loadDataReject(error);
        })'''

  for counter, file_ in enumerate(data_files):
    filename = file_.dstpath
    dirname = os.path.dirname(filename)
    basename = os.path.basename(filename)
    if file_.mode == 'embed':
      if not options.obj_output:
        # Embed (only needed when not generating object file output)
        data = base64_encode(utils.read_binary(file_.srcpath))
        code += "      var fileData%d = '%s';\n" % (counter, data)
        # canOwn this data in the filesystem (i.e. there is no need to create a copy in the FS layer).
        code += ("      Module['FS_createDataFile']('%s', '%s', atob(fileData%d), true, true, true);\n"
                 % (dirname, basename, counter))
    elif file_.mode == 'preload':
      # Preload
      metadata['files'].append({
        'filename': file_.dstpath,
        'start': file_.data_start,
        'end': file_.data_end,
      })
    else:
      assert 0

  if options.has_preloaded:
    if not options.lz4:
      # Get the big archive and split it up
      use_data = '''// Reuse the bytearray from the XHR as the source for file reads.
          for (var file of metadata['files']) {
            var name = file['filename'];
            var data = byteArray.subarray(file['start'], file['end']);
            %s
          }
          Module['removeRunDependency']('datafile_%s');''' % (finish_handler,
                                                              js_manipulation.escape_for_js_string(data_target))
    else:
      # LZ4FS usage
      temp = data_target + '.orig'
      shutil.move(data_target, temp)
      meta = shared.run_js_tool(utils.path_from_root('tools/lz4-compress.mjs'),
                                [temp, data_target], stdout=PIPE)
      os.unlink(temp)
      use_data = '''var compressedData = %s;
            compressedData['data'] = byteArray;
            assert(typeof Module['LZ4'] === 'object', 'LZ4 not present - was your app build with -sLZ4?');
            Module['LZ4'].loadPackage({ 'metadata': metadata, 'compressedData': compressedData }, %s);
            Module['removeRunDependency']('datafile_%s');''' % (meta, "true" if options.use_preload_plugins else "false", js_manipulation.escape_for_js_string(data_target))

    if options.export_es6:
      use_data += '\nloadDataResolve();'

    package_name = data_target
    remote_package_size = os.path.getsize(package_name)
    remote_package_name = os.path.basename(package_name)
    ret += '''
      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.substring(0, window.location.pathname.lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.substring(0, location.pathname.lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = '%s';
      var REMOTE_PACKAGE_BASE = '%s';
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;\n''' % (js_manipulation.escape_for_js_string(data_target), js_manipulation.escape_for_js_string(remote_package_name))
    metadata['remote_package_size'] = remote_package_size
    ret += "      var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];\n"

    if options.use_preload_cache:
      # Set the id to a hash of the preloaded data, so that caches survive over multiple builds
      # if the data has not changed.
      data = utils.read_binary(data_target)
      package_uuid = 'sha256-' + hashlib.sha256(data).hexdigest()
      metadata['package_uuid'] = str(package_uuid)

      code += r'''
        var PACKAGE_UUID = metadata['package_uuid'];
        var IDB_RO = "readonly";
        var IDB_RW = "readwrite";
        var DB_NAME = "''' + options.indexeddb_name + '''";
        var DB_VERSION = 1;
        var METADATA_STORE_NAME = 'METADATA';
        var PACKAGE_STORE_NAME = 'PACKAGES';

        async function openDatabase() {
          if (typeof indexedDB == 'undefined') {
            throw new Error('using IndexedDB to cache data can only be done on a web page or in a web worker');
          }
          return new Promise((resolve, reject) => {
            var openRequest = indexedDB.open(DB_NAME, DB_VERSION);
            openRequest.onupgradeneeded = (event) => {
              var db = /** @type {IDBDatabase} */ (event.target.result);

              if (db.objectStoreNames.contains(PACKAGE_STORE_NAME)) {
                db.deleteObjectStore(PACKAGE_STORE_NAME);
              }
              var packages = db.createObjectStore(PACKAGE_STORE_NAME);

              if (db.objectStoreNames.contains(METADATA_STORE_NAME)) {
                db.deleteObjectStore(METADATA_STORE_NAME);
              }
              var metadata = db.createObjectStore(METADATA_STORE_NAME);
            };
            openRequest.onsuccess = (event) => {
              var db = /** @type {IDBDatabase} */ (event.target.result);
              resolve(db);
            };
            openRequest.onerror = reject;
          });
        }

        // This is needed as chromium has a limit on per-entry files in IndexedDB
        // https://cs.chromium.org/chromium/src/content/renderer/indexed_db/webidbdatabase_impl.cc?type=cs&sq=package:chromium&g=0&l=177
        // https://cs.chromium.org/chromium/src/out/Debug/gen/third_party/blink/public/mojom/indexeddb/indexeddb.mojom.h?type=cs&sq=package:chromium&g=0&l=60
        // We set the chunk size to 64MB to stay well-below the limit
        var CHUNK_SIZE = 64 * 1024 * 1024;

        async function cacheRemotePackage(db, packageName, packageData, packageMeta) {
          var transactionPackages = db.transaction([PACKAGE_STORE_NAME], IDB_RW);
          var packages = transactionPackages.objectStore(PACKAGE_STORE_NAME);
          var chunkSliceStart = 0;
          var nextChunkSliceStart = 0;
          var chunkCount = Math.ceil(packageData.byteLength / CHUNK_SIZE);
          var finishedChunks = 0;

          return new Promise((resolve, reject) => {
            for (var chunkId = 0; chunkId < chunkCount; chunkId++) {
              nextChunkSliceStart += CHUNK_SIZE;
              var putPackageRequest = packages.put(
                packageData.slice(chunkSliceStart, nextChunkSliceStart),
                `package/${packageName}/${chunkId}`
              );
              chunkSliceStart = nextChunkSliceStart;
              putPackageRequest.onsuccess = (event) => {
                finishedChunks++;
                if (finishedChunks == chunkCount) {
                  var transaction_metadata = db.transaction(
                    [METADATA_STORE_NAME],
                    IDB_RW
                  );
                  var metadata = transaction_metadata.objectStore(METADATA_STORE_NAME);
                  var putMetadataRequest = metadata.put(
                    {
                      'uuid': packageMeta.uuid,
                      'chunkCount': chunkCount
                    },
                    `metadata/${packageName}`
                  );
                  putMetadataRequest.onsuccess = (event) => resolve(packageData);
                  putMetadataRequest.onerror = reject;
                }
              };
              putPackageRequest.onerror = reject;
            }
          });
        }

        /*
         * Check if there's a cached package, and if so whether it's the latest available.
         * Resolves to the cached metadata, or `null` if it is missing or out-of-date.
         */
        async function checkCachedPackage(db, packageName) {
          var transaction = db.transaction([METADATA_STORE_NAME], IDB_RO);
          var metadata = transaction.objectStore(METADATA_STORE_NAME);
          var getRequest = metadata.get(`metadata/${packageName}`);
          return new Promise((resolve, reject) => {
            getRequest.onsuccess = (event) => {
              var result = event.target.result;
              if (result && PACKAGE_UUID === result['uuid']) {
                resolve(result);
              } else {
                resolve(null);
              }
            }
            getRequest.onerror = reject;
          });
        }

        async function fetchCachedPackage(db, packageName, metadata) {
          var transaction = db.transaction([PACKAGE_STORE_NAME], IDB_RO);
          var packages = transaction.objectStore(PACKAGE_STORE_NAME);

          var chunksDone = 0;
          var totalSize = 0;
          var chunkCount = metadata['chunkCount'];
          var chunks = new Array(chunkCount);

          return new Promise((resolve, reject) => {
            for (var chunkId = 0; chunkId < chunkCount; chunkId++) {
              var getRequest = packages.get(`package/${packageName}/${chunkId}`);
              getRequest.onsuccess = (event) => {
                if (!event.target.result) {
                  reject(`CachedPackageNotFound for: ${packageName}`);
                  return;
                }
                // If there's only 1 chunk, there's nothing to concatenate it with so we can just return it now
                if (chunkCount == 1) {
                  resolve(event.target.result);
                } else {
                  chunksDone++;
                  totalSize += event.target.result.byteLength;
                  chunks.push(event.target.result);
                  if (chunksDone == chunkCount) {
                    if (chunksDone == 1) {
                      resolve(event.target.result);
                    } else {
                      var tempTyped = new Uint8Array(totalSize);
                      var byteOffset = 0;
                      for (var chunkId in chunks) {
                        var buffer = chunks[chunkId];
                        tempTyped.set(new Uint8Array(buffer), byteOffset);
                        byteOffset += buffer.byteLength;
                        buffer = undefined;
                      }
                      chunks = undefined;
                      resolve(tempTyped.buffer);
                      tempTyped = undefined;
                    }
                  }
                }
              };
              getRequest.onerror = reject;
            }
          });
        }\n'''

    # add Node.js support code, if necessary
    node_support_code = ''
    if options.support_node:
      node_support_code = '''
        if (isNode) {
          var contents = require('fs').readFileSync(packageName);
          return new Uint8Array(contents).buffer;
        }'''.strip()

    ret += '''
      async function fetchRemotePackage(packageName, packageSize) {
        %(node_support_code)s
        if (!Module['dataFileDownloads']) Module['dataFileDownloads'] = {};
        try {
          var response = await fetch(packageName);
        } catch (e) {
          throw new Error(`Network Error: ${packageName}`, {e});
        }
        if (!response.ok) {
          throw new Error(`${response.status}: ${response.url}`);
        }

        const chunks = [];
        const headers = response.headers;
        const total = Number(headers.get('Content-Length') || packageSize);
        let loaded = 0;

        Module['setStatus'] && Module['setStatus']('Downloading data...');
        const reader = response.body.getReader();

        while (1) {
          var {done, value} = await reader.read();
          if (done) break;
          chunks.push(value);
          loaded += value.length;
          Module['dataFileDownloads'][packageName] = {loaded, total};

          let totalLoaded = 0;
          let totalSize = 0;

          for (const download of Object.values(Module['dataFileDownloads'])) {
            totalLoaded += download.loaded;
            totalSize += download.total;
          }

          Module['setStatus'] && Module['setStatus'](`Downloading data... (${totalLoaded}/${totalSize})`);
        }

        const packageData = new Uint8Array(chunks.map((c) => c.length).reduce((a, b) => a + b, 0));
        let offset = 0;
        for (const chunk of chunks) {
          packageData.set(chunk, offset);
          offset += chunk.length;
        }
        return packageData.buffer;
      }\n''' % {'node_support_code': node_support_code}

    code += '''
      async function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData ' + arrayBuffer.constructor.name);
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        %s
      }
      Module['addRunDependency']('datafile_%s');\n''' % (use_data, js_manipulation.escape_for_js_string(data_target))
    # use basename because from the browser's point of view,
    # we need to find the datafile in the same dir as the html file

    code += '''
      if (!Module['preloadResults']) Module['preloadResults'] = {};\n'''

    if options.use_preload_cache:
      code += '''
        async function preloadFallback(error) {
          console.error(error);
          console.error('falling back to default preload behavior');
          processPackageData(await fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE));
        }

        try {
          var db = await openDatabase();
          var pkgMetadata = await checkCachedPackage(db, PACKAGE_PATH + PACKAGE_NAME);
          var useCached = !!pkgMetadata;
          Module['preloadResults'][PACKAGE_NAME] = {fromCache: useCached};
          if (useCached) {
            processPackageData(await fetchCachedPackage(db, PACKAGE_PATH + PACKAGE_NAME, pkgMetadata));
          } else {
            var packageData = await fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);
            try {
              processPackageData(await cacheRemotePackage(db, PACKAGE_PATH + PACKAGE_NAME, packageData, {uuid:PACKAGE_UUID}))
            } catch (error) {
              console.error(error);
              processPackageData(packageData);
            }
          }
        } catch(e) {
          await preloadFallback(e)%s;
        }

        Module['setStatus'] && Module['setStatus']('Downloading...');\n''' % catch_handler
    else:
      # Not using preload cache, so we might as well start the xhr ASAP,
      # potentially before JS parsing of the main codebase if it's after us.
      # Only tricky bit is the fetch is async, but also when runWithFS is called
      # is async, so we handle both orderings.
      ret += '''
      var fetchPromise;
      var fetched = Module['getPreloadedPackage']?.(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE);

      if (!fetched) {
        // Note that we don't use await here because we want to execute the
        // the rest of this function immediately.
        fetchPromise = fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE)%s;
      }\n''' % catch_handler

      code += '''
      Module['preloadResults'][PACKAGE_NAME] = {fromCache: false};
      if (!fetched) {
        fetched = await fetchPromise;
      }
      processPackageData(fetched);\n'''

  ret += '''
    async function runWithFS(Module) {\n'''
  ret += code
  ret += '''
    }
    if (Module['calledRun']) {
      runWithFS(Module)%s;
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module['preRun'].push(runWithFS); // FS is not initialized yet, wait for it
    }\n''' % catch_handler

  if options.separate_metadata:
    node_support_code = ''
    if options.support_node:
      node_support_code = '''
        if (isNode) {
          var contents = require('fs').readFileSync(metadataUrl, 'utf8');
          // The await here is needed, even though JSON.parse is a sync API.  It works
          // around a issue with `removeRunDependency` otherwise being called to early
          // on the metadata object.
          var json = await JSON.parse(contents);
          return loadPackage(json);
        }'''.strip()

    ret += '''
    Module['removeRunDependency']('%(metadata_file)s');
  }

  async function runMetaWithFS() {
    Module['addRunDependency']('%(metadata_file)s');
    var metadataUrl = Module['locateFile'] ? Module['locateFile']('%(metadata_file)s', '') : '%(metadata_file)s';
    %(node_support_code)s
    var response = await fetch(metadataUrl);
    if (!response.ok) {
      throw new Error(`${response.status}: ${response.url}`);
    }
    var json = await response.json();
    await loadPackage(json);
  }

  if (Module['calledRun']) {
    runMetaWithFS();
  } else {
    if (!Module['preRun']) Module['preRun'] = [];
    Module['preRun'].push(runMetaWithFS);
  }\n''' % {'node_support_code': node_support_code, 'metadata_file': os.path.basename(options.jsoutput + '.metadata')}
  else:
    ret += '''
    }
    loadPackage(%s);\n''' % json.dumps(metadata)

  if options.export_es6:
    ret += '''
  });
}
// END the loadDataFile function
'''
  else:
    ret += '''
  })();\n'''

  return ret


if __name__ == '__main__':
  sys.exit(main())
