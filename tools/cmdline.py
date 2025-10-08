# Copyright 2025 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

from tools.toolchain_profiler import ToolchainProfiler

import json
import logging
import os
import re
import shlex
import sys
from enum import Enum, auto, unique
from subprocess import PIPE

from tools import shared, utils, ports, diagnostics, config
from tools import cache, feature_matrix, colored_logger
from tools.shared import exit_with_error
from tools.settings import settings, user_settings, MEM_SIZE_SETTINGS
from tools.utils import removeprefix, read_file

SIMD_INTEL_FEATURE_TOWER = ['-msse', '-msse2', '-msse3', '-mssse3', '-msse4.1', '-msse4.2', '-msse4', '-mavx', '-mavx2']
SIMD_NEON_FLAGS = ['-mfpu=neon']
CLANG_FLAGS_WITH_ARGS = {
    '-MT', '-MF', '-MJ', '-MQ', '-D', '-U', '-o', '-x',
    '-Xpreprocessor', '-include', '-imacros', '-idirafter',
    '-iprefix', '-iwithprefix', '-iwithprefixbefore',
    '-isysroot', '-imultilib', '-A', '-isystem', '-iquote',
    '-install_name', '-compatibility_version', '-mllvm',
    '-current_version', '-I', '-L', '-include-pch', '-u',
    '-undefined', '-target', '-Xlinker', '-Xclang', '-z',
}
# These symbol names are allowed in INCOMING_MODULE_JS_API but are not part of the
# default set.
EXTRA_INCOMING_JS_API = [
  'fetchSettings',
]

logger = logging.getLogger('args')


@unique
class OFormat(Enum):
  # Output a relocatable object file.  We use this
  # today for `-r` and `-shared`.
  OBJECT = auto()
  WASM = auto()
  JS = auto()
  MJS = auto()
  HTML = auto()
  BARE = auto()


class EmccOptions:
  def __init__(self):
    self.target = ''
    self.output_file = None
    self.input_files = []
    self.no_minify = False
    self.post_link = False
    self.save_temps = False
    self.executable = False
    self.oformat = None
    self.requested_debug = None
    self.emit_symbol_map = False
    self.use_closure_compiler = None
    self.js_transform = None
    self.pre_js = [] # before all js
    self.post_js = [] # after all js
    self.extern_pre_js = [] # before all js, external to optimized code
    self.extern_post_js = [] # after all js, external to optimized code
    self.preload_files = []
    self.embed_files = []
    self.exclude_files = []
    self.ignore_dynamic_linking = False
    self.shell_path = None
    self.source_map_base = ''
    self.emit_tsd = ''
    self.emrun = False
    self.cpu_profiler = False
    self.memory_profiler = False
    self.use_preload_cache = False
    self.use_preload_plugins = False
    self.valid_abspaths = []
    # Specifies the line ending format to use for all generated text files.
    # Defaults to using the native EOL on each platform (\r\n on Windows, \n on
    # Linux & MacOS)
    self.output_eol = os.linesep
    self.no_entry = False
    self.shared = False
    self.relocatable = False
    self.reproduce = None
    self.syntax_only = False
    self.dash_c = False
    self.dash_E = False
    self.dash_S = False
    self.dash_M = False
    self.input_language = None
    self.nostdlib = False
    self.nostdlibxx = False
    self.nodefaultlibs = False
    self.nolibc = False
    self.nostartfiles = False
    self.sanitize_minimal_runtime = False
    self.sanitize = set()
    self.lib_dirs = []


def is_int(s):
  try:
    int(s)
    return True
  except ValueError:
    return False


def validate_arg_level(level_string, max_level, err_msg, clamp=False):
  try:
    level = int(level_string)
  except ValueError:
    exit_with_error(err_msg)
  if clamp:
    if level > max_level:
      logger.warning("optimization level '-O" + level_string + "' is not supported; using '-O" + str(max_level) + "' instead")
      level = max_level
  if not 0 <= level <= max_level:
    exit_with_error(err_msg)
  return level


def version_string():
  # if the emscripten folder is not a git repo, don't run git show - that can
  # look up and find the revision in a parent directory that is a git repo
  revision_suffix = ''
  if os.path.exists(utils.path_from_root('.git')):
    git_rev = shared.run_process(
      ['git', 'rev-parse', 'HEAD'],
      stdout=PIPE, stderr=PIPE, cwd=utils.path_from_root()).stdout.strip()
    revision_suffix = ' (%s)' % git_rev
  elif os.path.exists(utils.path_from_root('emscripten-revision.txt')):
    rev = read_file(utils.path_from_root('emscripten-revision.txt')).strip()
    revision_suffix = ' (%s)' % rev
  return f'emcc (Emscripten gcc/clang-like replacement + linker emulating GNU ld) {utils.EMSCRIPTEN_VERSION}{revision_suffix}'


def is_valid_abspath(options, path_name):
  # Any path that is underneath the emscripten repository root must be ok.
  if utils.normalize_path(path_name).startswith(utils.normalize_path(utils.path_from_root())):
    return True

  def in_directory(root, child):
    # make both path absolute
    root = os.path.realpath(root)
    child = os.path.realpath(child)

    # return true, if the common prefix of both is equal to directory
    # e.g. /a/b/c/d.rst and directory is /a/b, the common prefix is /a/b
    return os.path.commonprefix([root, child]) == root

  for valid_abspath in options.valid_abspaths:
    if in_directory(valid_abspath, path_name):
      return True
  return False


def is_dash_s_for_emcc(args, i):
  # -s OPT=VALUE or -s OPT or -sOPT are all interpreted as emscripten flags.
  # -s by itself is a linker option (alias for --strip-all)
  if args[i] == '-s':
    if len(args) <= i + 1:
      return False
    arg = args[i + 1]
  else:
    arg = removeprefix(args[i], '-s')
  arg = arg.split('=')[0]
  return arg.isidentifier() and arg.isupper()


def parse_s_args(args):
  settings_changes = []
  for i in range(len(args)):
    if args[i].startswith('-s'):
      if is_dash_s_for_emcc(args, i):
        if args[i] == '-s':
          key = args[i + 1]
          args[i + 1] = ''
        else:
          key = removeprefix(args[i], '-s')
        args[i] = ''

        # If not = is specified default to 1
        if '=' not in key:
          key += '=1'

        # Special handling of browser version targets. A version -1 means that the specific version
        # is not supported at all. Replace those with INT32_MAX to make it possible to compare e.g.
        # #if MIN_FIREFOX_VERSION < 68
        if re.match(r'MIN_.*_VERSION(=.*)?', key):
          try:
            if int(key.split('=')[1]) < 0:
              key = key.split('=')[0] + '=0x7FFFFFFF'
          except Exception:
            pass

        settings_changes.append(key)

  newargs = [a for a in args if a]
  return (settings_changes, newargs)


def parse_args(newargs):  # noqa: C901, PLR0912, PLR0915
  """Future modifications should consider refactoring to reduce complexity.

  * The McCabe cyclomatiic complexity is currently 117 vs 10 recommended.
  * There are currently 115 branches vs 12 recommended.
  * There are currently 302 statements vs 50 recommended.

  To revalidate these numbers, run `ruff check --select=C901,PLR091`.
  """
  options = EmccOptions()
  settings_changes = []
  user_js_defines = []
  should_exit = False
  skip = False

  for i in range(len(newargs)):
    if skip:
      skip = False
      continue

    # Support legacy '--bind' flag, by mapping to `-lembind` which now
    # has the same effect
    if newargs[i] == '--bind':
      newargs[i] = '-lembind'

    arg = newargs[i]
    arg_value = None

    if arg in CLANG_FLAGS_WITH_ARGS:
      # Ignore the next argument rather than trying to parse it.  This is needed
      # because that next arg could, for example, start with `-o` and we don't want
      # to confuse that with a normal `-o` flag.
      skip = True
    elif arg == '-s' and is_dash_s_for_emcc(newargs, i):
      skip = True

    def check_flag(value):
      # Check for and consume a flag
      if arg == value:
        newargs[i] = ''
        return True
      return False

    def check_arg(name):
      nonlocal arg_value
      if arg.startswith(name) and '=' in arg:
        arg_value = arg.split('=', 1)[1]
        newargs[i] = ''
        return True
      if arg == name:
        if len(newargs) <= i + 1:
          exit_with_error(f"option '{arg}' requires an argument")
        arg_value = newargs[i + 1]
        newargs[i] = ''
        newargs[i + 1] = ''
        return True
      return False

    def consume_arg():
      nonlocal arg_value
      assert arg_value is not None
      rtn = arg_value
      arg_value = None
      return rtn

    def consume_arg_file():
      name = consume_arg()
      if not os.path.isfile(name):
        exit_with_error("'%s': file not found: '%s'" % (arg, name))
      return name

    if arg.startswith('-O'):
      # Let -O default to -O2, which is what gcc does.
      requested_level = removeprefix(arg, '-O') or '2'
      if requested_level == 's':
        requested_level = 2
        settings.SHRINK_LEVEL = 1
      elif requested_level == 'z':
        requested_level = 2
        settings.SHRINK_LEVEL = 2
      elif requested_level == 'g':
        requested_level = 1
        settings.SHRINK_LEVEL = 0
        settings.DEBUG_LEVEL = max(settings.DEBUG_LEVEL, 1)
      elif requested_level == 'fast':
        # TODO(https://github.com/emscripten-core/emscripten/issues/21497):
        # If we ever map `-ffast-math` to `wasm-opt --fast-math` then
        # then we should enable that too here.
        requested_level = 3
        settings.SHRINK_LEVEL = 0
      else:
        settings.SHRINK_LEVEL = 0
      settings.OPT_LEVEL = validate_arg_level(requested_level, 3, 'invalid optimization level: ' + arg, clamp=True)
    elif check_arg('--js-opts'):
      logger.warning('--js-opts ignored when using llvm backend')
      consume_arg()
    elif check_arg('--llvm-opts'):
      diagnostics.warning('deprecated', '--llvm-opts is deprecated.  All non-emcc args are passed through to clang.')
    elif arg.startswith('-flto'):
      if '=' in arg:
        settings.LTO = arg.split('=')[1]
      else:
        settings.LTO = 'full'
    elif arg == "-fno-lto":
      settings.LTO = 0
    elif arg == "--save-temps":
      options.save_temps = True
    elif check_arg('--llvm-lto'):
      logger.warning('--llvm-lto ignored when using llvm backend')
      consume_arg()
    elif check_arg('--closure-args'):
      args = consume_arg()
      settings.CLOSURE_ARGS += shlex.split(args)
    elif check_arg('--closure'):
      options.use_closure_compiler = int(consume_arg())
    elif check_arg('--js-transform'):
      options.js_transform = consume_arg()
    elif check_arg('--reproduce'):
      options.reproduce = consume_arg()
    elif check_arg('--pre-js'):
      options.pre_js.append(consume_arg_file())
    elif check_arg('--post-js'):
      options.post_js.append(consume_arg_file())
    elif check_arg('--extern-pre-js'):
      options.extern_pre_js.append(consume_arg_file())
    elif check_arg('--extern-post-js'):
      options.extern_post_js.append(consume_arg_file())
    elif check_arg('--compiler-wrapper'):
      config.COMPILER_WRAPPER = consume_arg()
    elif check_flag('--post-link'):
      options.post_link = True
    elif check_arg('--oformat'):
      formats = [f.lower() for f in OFormat.__members__]
      fmt = consume_arg()
      if fmt not in formats:
        exit_with_error('invalid output format: `%s` (must be one of %s)' % (fmt, formats))
      options.oformat = getattr(OFormat, fmt.upper())
    elif check_arg('--minify'):
      arg = consume_arg()
      if arg != '0':
        exit_with_error('0 is the only supported option for --minify; 1 has been deprecated')
      options.no_minify = True
    elif arg.startswith('-g'):
      options.requested_debug = arg
      requested_level = removeprefix(arg, '-g') or '3'
      if is_int(requested_level):
        # the -gX value is the debug level (-g1, -g2, etc.)
        settings.DEBUG_LEVEL = validate_arg_level(requested_level, 4, 'invalid debug level: ' + arg)
        if settings.DEBUG_LEVEL == 0:
          # Set these explicitly so -g0 overrides previous -g on the cmdline
          settings.GENERATE_DWARF = 0
          settings.GENERATE_SOURCE_MAP = 0
          settings.EMIT_NAME_SECTION = 0
        elif settings.DEBUG_LEVEL > 1:
          settings.EMIT_NAME_SECTION = 1
        # if we don't need to preserve LLVM debug info, do not keep this flag
        # for clang
        if (settings.DEBUG_LEVEL < 3 and not
            (settings.GENERATE_SOURCE_MAP or settings.SEPARATE_DWARF)):
          newargs[i] = '-g0'
        else:
          # for 3+, report -g3 to clang as -g4 etc. are not accepted
          newargs[i] = '-g3'
          if settings.DEBUG_LEVEL == 3:
            settings.GENERATE_DWARF = 1
          if settings.DEBUG_LEVEL == 4:
            settings.GENERATE_SOURCE_MAP = 1
            diagnostics.warning('deprecated', 'please replace -g4 with -gsource-map')
      else:
        if requested_level.startswith('force_dwarf'):
          exit_with_error('gforce_dwarf was a temporary option and is no longer necessary (use -g)')
        elif requested_level.startswith('separate-dwarf'):
          # emit full DWARF but also emit it in a file on the side
          newargs[i] = '-g'
          # if a file is provided, use that; otherwise use the default location
          # (note that we do not know the default location until all args have
          # been parsed, so just note True for now).
          if requested_level != 'separate-dwarf':
            if not requested_level.startswith('separate-dwarf=') or requested_level.count('=') != 1:
              exit_with_error('invalid -gseparate-dwarf=FILENAME notation')
            settings.SEPARATE_DWARF = requested_level.split('=')[1]
          else:
            settings.SEPARATE_DWARF = True
          settings.GENERATE_DWARF = 1
          settings.DEBUG_LEVEL = 3
        elif requested_level in ['source-map', 'source-map=inline']:
          settings.GENERATE_SOURCE_MAP = 1 if requested_level == 'source-map' else 2
          newargs[i] = '-g'
        elif requested_level == 'z':
          # Ignore `-gz`.  We don't support debug info compression.
          continue
        else:
          # Other non-integer levels (e.g. -gline-tables-only or -gdwarf-5) are
          # usually clang flags that emit DWARF. So we pass them through to
          # clang and make the emscripten code treat it like any other DWARF.
          settings.GENERATE_DWARF = 1
          settings.EMIT_NAME_SECTION = 1
          settings.DEBUG_LEVEL = 3
    elif check_flag('-profiling') or check_flag('--profiling'):
      settings.DEBUG_LEVEL = max(settings.DEBUG_LEVEL, 2)
      settings.EMIT_NAME_SECTION = 1
    elif check_flag('-profiling-funcs') or check_flag('--profiling-funcs'):
      settings.EMIT_NAME_SECTION = 1
    elif newargs[i] == '--tracing' or newargs[i] == '--memoryprofiler':
      if newargs[i] == '--memoryprofiler':
        options.memory_profiler = True
      newargs[i] = ''
      settings_changes.append('EMSCRIPTEN_TRACING=1')
    elif check_flag('--emit-symbol-map'):
      options.emit_symbol_map = True
      settings.EMIT_SYMBOL_MAP = 1
    elif check_arg('--emit-minification-map'):
      settings.MINIFICATION_MAP = consume_arg()
    elif check_arg('--embed-file'):
      options.embed_files.append(consume_arg())
    elif check_arg('--preload-file'):
      options.preload_files.append(consume_arg())
    elif check_arg('--exclude-file'):
      options.exclude_files.append(consume_arg())
    elif check_flag('--use-preload-cache'):
      options.use_preload_cache = True
    elif check_flag('--no-heap-copy'):
      diagnostics.warning('legacy-settings', 'ignoring legacy flag --no-heap-copy (that is the only mode supported now)')
    elif check_flag('--use-preload-plugins'):
      options.use_preload_plugins = True
    elif check_flag('--ignore-dynamic-linking'):
      options.ignore_dynamic_linking = True
    elif arg == '-v':
      shared.PRINT_SUBPROCS = True
    elif arg == '-###':
      shared.SKIP_SUBPROCS = True
    elif check_arg('--shell-file'):
      options.shell_path = consume_arg_file()
    elif check_arg('--source-map-base'):
      options.source_map_base = consume_arg()
    elif check_arg('--embind-emit-tsd'):
      diagnostics.warning('deprecated', '--embind-emit-tsd is deprecated.  Use --emit-tsd instead.')
      options.emit_tsd = consume_arg()
    elif check_arg('--emit-tsd'):
      options.emit_tsd = consume_arg()
    elif check_flag('--no-entry'):
      options.no_entry = True
    elif check_flag('--remove-duplicates'):
      diagnostics.warning('legacy-settings', '--remove-duplicates is deprecated as it is no longer needed. If you cannot link without it, file a bug with a testcase')
    elif check_flag('--jcache'):
      logger.error('jcache is no longer supported')
    elif check_arg('--cache'):
      config.CACHE = os.path.abspath(consume_arg())
      cache.setup()
      # Ensure child processes share the same cache (e.g. when using emcc to compiler system
      # libraries)
      os.environ['EM_CACHE'] = config.CACHE
    elif check_flag('--clear-cache'):
      logger.info('clearing cache as requested by --clear-cache: `%s`', cache.cachedir)
      cache.erase()
      shared.perform_sanity_checks() # this is a good time for a sanity check
      should_exit = True
    elif check_flag('--clear-ports'):
      logger.info('clearing ports and cache as requested by --clear-ports')
      ports.clear()
      cache.erase()
      shared.perform_sanity_checks() # this is a good time for a sanity check
      should_exit = True
    elif check_flag('--check'):
      print(version_string(), file=sys.stderr)
      shared.check_sanity(force=True)
      should_exit = True
    elif check_flag('--show-ports'):
      ports.show_ports()
      should_exit = True
    elif check_arg('--memory-init-file'):
      exit_with_error('--memory-init-file is no longer supported')
    elif check_flag('--proxy-to-worker'):
      settings_changes.append('PROXY_TO_WORKER=1')
    elif check_arg('--valid-abspath'):
      options.valid_abspaths.append(consume_arg())
    elif check_flag('--separate-asm'):
      exit_with_error('cannot --separate-asm with the wasm backend, since not emitting asm.js')
    elif arg.startswith(('-I', '-L')):
      path_name = arg[2:]
      # Look for '/' explicitly so that we can also diagnose identically if -I/foo/bar is passed on Windows.
      # Python since 3.13 does not treat '/foo/bar' as an absolute path on Windows.
      if (path_name.startswith('/') or os.path.isabs(path_name)) and not is_valid_abspath(options, path_name):
        # Of course an absolute path to a non-system-specific library or header
        # is fine, and you can ignore this warning. The danger are system headers
        # that are e.g. x86 specific and non-portable. The emscripten bundled
        # headers are modified to be portable, local system ones are generally not.
        diagnostics.warning(
            'absolute-paths', f'-I or -L of an absolute path "{arg}" '
            'encountered. If this is to a local system header/library, it may '
            'cause problems (local system files make sense for compiling natively '
            'on your system, but not necessarily to JavaScript).')
      if arg.startswith('-L'):
        options.lib_dirs.append(path_name)
    elif check_flag('--emrun'):
      options.emrun = True
    elif check_flag('--cpuprofiler'):
      options.cpu_profiler = True
    elif check_flag('--threadprofiler'):
      settings_changes.append('PTHREADS_PROFILING=1')
    elif arg in ('-fcolor-diagnostics', '-fdiagnostics-color', '-fdiagnostics-color=always'):
      colored_logger.enable(force=True)
    elif arg in ('-fno-color-diagnostics', '-fno-diagnostics-color', '-fdiagnostics-color=never'):
      colored_logger.disable()
    elif arg == '-fno-exceptions':
      settings.DISABLE_EXCEPTION_CATCHING = 1
      settings.DISABLE_EXCEPTION_THROWING = 1
      settings.WASM_EXCEPTIONS = 0
    elif arg == '-mbulk-memory':
      feature_matrix.enable_feature(feature_matrix.Feature.BULK_MEMORY,
                                    '-mbulk-memory',
                                    override=True)
    elif arg == '-mno-bulk-memory':
      feature_matrix.disable_feature(feature_matrix.Feature.BULK_MEMORY)
    elif arg == '-msign-ext':
      feature_matrix.enable_feature(feature_matrix.Feature.SIGN_EXT,
                                    '-msign-ext',
                                    override=True)
    elif arg == '-mno-sign-ext':
      feature_matrix.disable_feature(feature_matrix.Feature.SIGN_EXT)
    elif arg == '-mnontrappting-fptoint':
      feature_matrix.enable_feature(feature_matrix.Feature.NON_TRAPPING_FPTOINT,
                                    '-mnontrapping-fptoint',
                                    override=True)
    elif arg == '-mno-nontrapping-fptoint':
      feature_matrix.disable_feature(feature_matrix.Feature.NON_TRAPPING_FPTOINT)
    elif arg == '-fexceptions':
      # TODO Currently -fexceptions only means Emscripten EH. Switch to wasm
      # exception handling by default when -fexceptions is given when wasm
      # exception handling becomes stable.
      settings.DISABLE_EXCEPTION_THROWING = 0
      settings.DISABLE_EXCEPTION_CATCHING = 0
    elif arg == '-fwasm-exceptions':
      settings.WASM_EXCEPTIONS = 1
    elif arg == '-fignore-exceptions':
      settings.DISABLE_EXCEPTION_CATCHING = 1
    elif check_arg('--default-obj-ext'):
      exit_with_error('--default-obj-ext is no longer supported by emcc')
    elif arg.startswith('-fsanitize=cfi'):
      exit_with_error('emscripten does not currently support -fsanitize=cfi')
    elif check_arg('--output_eol') or check_arg('--output-eol'):
      style = consume_arg()
      if style.lower() == 'windows':
        options.output_eol = '\r\n'
      elif style.lower() == 'linux':
        options.output_eol = '\n'
      else:
        exit_with_error(f'invalid value for --output-eol: `{style}`')
    # Record PTHREADS setting because it controls whether --shared-memory is passed to lld
    elif arg == '-pthread':
      settings.PTHREADS = 1
      # Also set the legacy setting name, in case use JS code depends on it.
      settings.USE_PTHREADS = 1
    elif arg == '-no-pthread':
      settings.PTHREADS = 0
      # Also set the legacy setting name, in case use JS code depends on it.
      settings.USE_PTHREADS = 0
    elif arg == '-pthreads':
      exit_with_error('unrecognized command-line option `-pthreads`; did you mean `-pthread`?')
    elif arg == '-fno-rtti':
      settings.USE_RTTI = 0
    elif arg == '-frtti':
      settings.USE_RTTI = 1
    elif arg.startswith('-jsD'):
      key = removeprefix(arg, '-jsD')
      if '=' in key:
        key, value = key.split('=')
      else:
        value = '1'
      if key in settings.keys():
        exit_with_error(f'{arg}: cannot change built-in settings values with a -jsD directive. Pass -s{key}={value} instead!')
      user_js_defines += [(key, value)]
      newargs[i] = ''
    elif check_flag('-shared'):
      options.shared = True
    elif check_flag('-r'):
      options.relocatable = True
    elif arg.startswith('-o'):
      options.output_file = removeprefix(arg, '-o')
    elif check_arg('-target') or check_arg('--target'):
      options.target = consume_arg()
      if options.target not in ('wasm32', 'wasm64', 'wasm64-unknown-emscripten', 'wasm32-unknown-emscripten'):
        exit_with_error(f'unsupported target: {options.target} (emcc only supports wasm64-unknown-emscripten and wasm32-unknown-emscripten)')
    elif check_arg('--use-port'):
      ports.handle_use_port_arg(settings, consume_arg())
    elif arg in ('-c', '--precompile'):
      options.dash_c = True
    elif arg == '-S':
      options.dash_S = True
    elif arg == '-E':
      options.dash_E = True
    elif arg in ('-M', '-MM'):
      options.dash_M = True
    elif arg.startswith('-x'):
      # TODO(sbc): Handle multiple -x flags on the same command line
      options.input_language = arg
    elif arg == '-fsyntax-only':
      options.syntax_only = True
    elif arg in SIMD_INTEL_FEATURE_TOWER or arg in SIMD_NEON_FLAGS:
      # SSEx is implemented on top of SIMD128 instruction set, but do not pass SSE flags to LLVM
      # so it won't think about generating native x86 SSE code.
      newargs[i] = ''
    elif arg == '-nostdlib':
      options.nostdlib = True
    elif arg == '-nostdlibxx':
      options.nostdlibxx = True
    elif arg == '-nodefaultlibs':
      options.nodefaultlibs = True
    elif arg == '-nolibc':
      options.nolibc = True
    elif arg == '-nostartfiles':
      options.nostartfiles = True
    elif arg == '-fsanitize-minimal-runtime':
      options.sanitize_minimal_runtime = True
    elif arg.startswith('-fsanitize='):
      options.sanitize.update(arg.split('=', 1)[1].split(','))
    elif arg.startswith('-fno-sanitize='):
      options.sanitize.difference_update(arg.split('=', 1)[1].split(','))
    elif arg and (arg == '-' or not arg.startswith('-')):
      options.input_files.append(arg)

  if should_exit:
    sys.exit(0)

  newargs = [a for a in newargs if a]
  return options, settings_changes, user_js_defines, newargs


def expand_byte_size_suffixes(value):
  """Given a string with KB/MB size suffixes, such as "32MB", computes how
  many bytes that is and returns it as an integer.
  """
  value = value.strip()
  match = re.match(r'^(\d+)\s*([kmgt]?b)?$', value, re.I)
  if not match:
    exit_with_error("invalid byte size `%s`.  Valid suffixes are: kb, mb, gb, tb" % value)
  value, suffix = match.groups()
  value = int(value)
  if suffix:
    size_suffixes = {suffix: 1024 ** i for i, suffix in enumerate(['b', 'kb', 'mb', 'gb', 'tb'])}
    value *= size_suffixes[suffix.lower()]
  return value


def parse_symbol_list_file(contents):
  """Parse contents of one-symbol-per-line response file.  This format can by used
  with, for example, -sEXPORTED_FUNCTIONS=@filename and avoids the need for any
  kind of quoting or escaping.
  """
  values = contents.splitlines()
  return [v.strip() for v in values if not v.startswith('#')]


def parse_value(text, expected_type):
  # Note that using response files can introduce whitespace, if the file
  # has a newline at the end. For that reason, we rstrip() in relevant
  # places here.
  def parse_string_value(text):
    first = text[0]
    if first in {"'", '"'}:
      text = text.rstrip()
      if text[-1] != text[0] or len(text) < 2:
         raise ValueError(f'unclosed quoted string. expected final character to be "{text[0]}" and length to be greater than 1 in "{text[0]}"')
      return text[1:-1]
    return text

  def parse_string_list_members(text):
    sep = ','
    values = text.split(sep)
    result = []
    index = 0
    while True:
      current = values[index].lstrip() # Cannot safely rstrip for cases like: "HERE-> ,"
      if not len(current):
        raise ValueError('empty value in string list')
      first = current[0]
      if first not in {"'", '"'}:
        result.append(current.rstrip())
      else:
        start = index
        while True: # Continue until closing quote found
          if index >= len(values):
            raise ValueError(f"unclosed quoted string. expected final character to be '{first}' in '{values[start]}'")
          new = values[index].rstrip()
          if new and new[-1] == first:
            if start == index:
              result.append(current.rstrip()[1:-1])
            else:
              result.append((current + sep + new)[1:-1])
            break
          else:
            current += sep + values[index]
            index += 1

      index += 1
      if index >= len(values):
        break
    return result

  def parse_string_list(text):
    text = text.rstrip()
    if text and text[0] == '[':
      if text[-1] != ']':
        raise ValueError('unterminated string list. expected final character to be "]"')
      text = text[1:-1]
    if text.strip() == "":
      return []
    return parse_string_list_members(text)

  if expected_type == list or (text and text[0] == '['):
    # if json parsing fails, we fall back to our own parser, which can handle a few
    # simpler syntaxes
    try:
      parsed = json.loads(text)
    except ValueError:
      return parse_string_list(text)

    # if we succeeded in parsing as json, check some properties of it before returning
    if type(parsed) not in (str, list):
      raise ValueError(f'settings must be strings or lists (not ${type(parsed)})')
    if type(parsed) is list:
      for elem in parsed:
        if type(elem) is not str:
          raise ValueError(f'list members in settings must be strings (not ${type(elem)})')

    return parsed

  if expected_type == float:
    try:
      return float(text)
    except ValueError:
      pass

  try:
    if text.startswith('0x'):
      base = 16
    else:
      base = 10
    return int(text, base)
  except ValueError:
    return parse_string_value(text)


def apply_user_settings():
  """Take a map of users settings {NAME: VALUE} and apply them to the global
  settings object.
  """

  # Stash a copy of all available incoming APIs before the user can potentially override it
  settings.ALL_INCOMING_MODULE_JS_API = settings.INCOMING_MODULE_JS_API + EXTRA_INCOMING_JS_API

  for key, value in user_settings.items():
    if key in settings.internal_settings:
      exit_with_error('%s is an internal setting and cannot be set from command line', key)

    # map legacy settings which have aliases to the new names
    # but keep the original key so errors are correctly reported via the `setattr` below
    user_key = key
    if key in settings.legacy_settings and key in settings.alt_names:
      key = settings.alt_names[key]

    # In those settings fields that represent amount of memory, translate suffixes to multiples of 1024.
    if key in MEM_SIZE_SETTINGS:
      value = str(expand_byte_size_suffixes(value))

    filename = None
    if value and value[0] == '@':
      filename = removeprefix(value, '@')
      if not os.path.isfile(filename):
        exit_with_error('%s: file not found parsing argument: %s=%s' % (filename, key, value))
      value = read_file(filename).strip()
    else:
      value = value.replace('\\', '\\\\')

    expected_type = settings.types.get(key)

    if filename and expected_type == list and value.strip()[0] != '[':
      # Prefer simpler one-line-per value parser
      value = parse_symbol_list_file(value)
    else:
      try:
        value = parse_value(value, expected_type)
      except Exception as e:
        exit_with_error(f'error parsing "-s" setting "{key}={value}": {e}')

    setattr(settings, user_key, value)

    if key == 'EXPORTED_FUNCTIONS':
      # used for warnings in emscripten.py
      settings.USER_EXPORTS = settings.EXPORTED_FUNCTIONS.copy()

    # TODO(sbc): Remove this legacy way.
    if key == 'WASM_OBJECT_FILES':
      settings.LTO = 0 if value else 'full'

    if key == 'JSPI':
      settings.ASYNCIFY = 2
    if key == 'JSPI_IMPORTS':
      settings.ASYNCIFY_IMPORTS = value
    if key == 'JSPI_EXPORTS':
      settings.ASYNCIFY_EXPORTS = value


def normalize_boolean_setting(name, value):
  # boolean NO_X settings are aliases for X
  # (note that *non*-boolean setting values have special meanings,
  # and we can't just flip them, so leave them as-is to be
  # handled in a special way later)
  if name.startswith('NO_') and value in ('0', '1'):
    name = removeprefix(name, 'NO_')
    value = str(1 - int(value))
  return name, value


@ToolchainProfiler.profile()
def parse_arguments(args):
  newargs = list(args)

  # Scan and strip emscripten specific cmdline warning flags.
  # This needs to run before other cmdline flags have been parsed, so that
  # warnings are properly printed during arg parse.
  newargs = diagnostics.capture_warnings(newargs)

  if not diagnostics.is_enabled('deprecated'):
    settings.WARN_DEPRECATED = 0

  for i in range(len(newargs)):
    if newargs[i] in ('-l', '-L', '-I', '-z', '--js-library', '-o', '-x', '-u'):
      # Scan for flags that can be written as either one or two arguments
      # and normalize them to the single argument form.
      if newargs[i] == '--js-library':
        newargs[i] += '='
      if len(newargs) <= i + 1:
        exit_with_error(f"option '{newargs[i]}' requires an argument")
      newargs[i] += newargs[i + 1]
      newargs[i + 1] = ''

  options, settings_changes, user_js_defines, newargs = parse_args(newargs)

  if options.post_link or options.oformat == OFormat.BARE:
    diagnostics.warning('experimental', '--oformat=bare/--post-link are experimental and subject to change.')

  explicit_settings_changes, newargs = parse_s_args(newargs)
  settings_changes += explicit_settings_changes

  for s in settings_changes:
    key, value = s.split('=', 1)
    key, value = normalize_boolean_setting(key, value)
    user_settings[key] = value

  # STRICT is used when applying settings so it needs to be applied first before
  # calling `apply_user_settings`.
  strict_cmdline = user_settings.get('STRICT')
  if strict_cmdline:
    settings.STRICT = int(strict_cmdline)

  # Apply user -jsD settings
  for s in user_js_defines:
    settings[s[0]] = s[1]

  # Apply -s settings in newargs here (after optimization levels, so they can override them)
  apply_user_settings()

  return options, newargs
