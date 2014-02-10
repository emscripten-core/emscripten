import os, json, logging
import shared
from tools.shared import execute

def calculate(temp_files, in_temp, stdout, stderr):
  # Check if we need to include some libraries that we compile. (We implement libc ourselves in js, but
  # compile a malloc implementation and stdlibc++.)

  def read_symbols(path, exclude=None):
    symbols = map(lambda line: line.strip().split(' ')[1], open(path).readlines())
    if exclude:
      symbols = filter(lambda symbol: symbol not in exclude, symbols)
    return set(symbols)

  lib_opts = ['-O2']

  # XXX We also need to add libc symbols that use malloc, for example strdup. It's very rare to use just them and not
  #     a normal malloc symbol (like free, after calling strdup), so we haven't hit this yet, but it is possible.
  libc_symbols = read_symbols(shared.path_from_root('system', 'lib', 'libc.symbols'))
  libcextra_symbols = read_symbols(shared.path_from_root('system', 'lib', 'libcextra.symbols'))
  libcxx_symbols = read_symbols(shared.path_from_root('system', 'lib', 'libcxx', 'symbols'), exclude=libc_symbols)
  libcxxabi_symbols = read_symbols(shared.path_from_root('system', 'lib', 'libcxxabi', 'symbols'), exclude=libc_symbols)
  gl_symbols = read_symbols(shared.path_from_root('system', 'lib', 'gl.symbols'))

  # XXX we should disable EMCC_DEBUG when building libs, just like in the relooper

  def build_libc(lib_filename, files):
    o_s = []
    prev_cxx = os.environ.get('EMMAKEN_CXX')
    if prev_cxx: os.environ['EMMAKEN_CXX'] = ''
    musl_internal_includes = shared.path_from_root('system', 'lib', 'libc', 'musl', 'src', 'internal')
    for src in files:
      o = in_temp(os.path.basename(src) + '.o')
      execute([shared.PYTHON, shared.EMCC, shared.path_from_root('system', 'lib', src), '-o', o, '-I', musl_internal_includes] + lib_opts, stdout=stdout, stderr=stderr)
      o_s.append(o)
    if prev_cxx: os.environ['EMMAKEN_CXX'] = prev_cxx
    shared.Building.link(o_s, in_temp(lib_filename))
    return in_temp(lib_filename)

  def build_libcxx(src_dirname, lib_filename, files):
    o_s = []
    for src in files:
      o = in_temp(src + '.o')
      srcfile = shared.path_from_root(src_dirname, src)
      execute([shared.PYTHON, shared.EMXX, srcfile, '-o', o, '-std=c++11'] + lib_opts, stdout=stdout, stderr=stderr)
      o_s.append(o)
    shared.Building.link(o_s, in_temp(lib_filename))
    return in_temp(lib_filename)

  # libc
  def create_libc():
    logging.debug(' building libc for cache')
    libc_files = [
      'dlmalloc.c',
      os.path.join('libcxx', 'new.cpp'),
    ]
    musl_files = [
      ['internal', [
       'floatscan.c',
       'shgetc.c',
      ]],
      ['math', [
       'scalbn.c',
       'scalbnl.c',
      ]],
      ['stdio', [
       '__overflow.c',
       '__toread.c',
       '__towrite.c',
       '__uflow.c',
      ]],
      ['stdlib', [
       'atof.c',
       'strtod.c',
      ]],
      ['string', [
       'memcmp.c',
       'strcasecmp.c',
       'strcmp.c',
       'strncasecmp.c',
       'strncmp.c',
      ]]
    ]
    for directory, sources in musl_files:
      libc_files += [os.path.join('libc', 'musl', 'src', directory, source) for source in sources]
    return build_libc('libc.bc', libc_files)

  def apply_libc(need):
    # libc needs some sign correction. # If we are in mode 0, switch to 2. We will add our lines
    try:
      if shared.Settings.CORRECT_SIGNS == 0: raise Exception('we need to change to 2')
    except: # we fail if equal to 0 - so we need to switch to 2 - or if CORRECT_SIGNS is not even in Settings
      shared.Settings.CORRECT_SIGNS = 2
    if shared.Settings.CORRECT_SIGNS == 2:
      shared.Settings.CORRECT_SIGNS_LINES = [shared.path_from_root('src', 'dlmalloc.c') + ':' + str(i+4) for i in [4816, 4191, 4246, 4199, 4205, 4235, 4227]]
    # If we are in mode 1, we are correcting everything anyhow. If we are in mode 3, we will be corrected
    # so all is well anyhow too.
    return True

  # libcextra
  def create_libcextra():
    logging.debug('building libcextra for cache')
    musl_files = [
       ['ctype', [
        'iswalnum.c',
        'iswalpha.c',
        'iswblank.c',
        'iswcntrl.c',
        'iswctype.c',
        'iswdigit.c',
        'iswgraph.c',
        'iswlower.c',
        'iswprint.c',
        'iswpunct.c',
        'iswspace.c',
        'iswupper.c',
        'iswxdigit.c',
        'towctrans.c',
        'wcswidth.c',
        'wctrans.c',
        'wcwidth.c',
       ]],
       ['internal', [
        'intscan.c',
       ]],
       ['legacy', [
        'err.c',
       ]],
       ['locale', [
        'iconv.c',
        'iswalnum_l.c',
        'iswalpha_l.c',
        'iswblank_l.c',
        'iswcntrl_l.c',
        'iswctype_l.c',
        'iswdigit_l.c',
        'iswgraph_l.c',
        'iswlower_l.c',
        'iswprint_l.c',
        'iswpunct_l.c',
        'iswspace_l.c',
        'iswupper_l.c',
        'iswxdigit_l.c',
        'strcoll.c',
        'strcasecmp_l.c',
        'strfmon.c',
        'strncasecmp_l.c',
        'strxfrm.c',
        'towctrans_l.c',
        'towlower_l.c',
        'towupper_l.c',
        'wcscoll.c',
        'wcscoll_l.c',
        'wcsxfrm.c',
        'wcsxfrm_l.c',
        'wctrans_l.c',
        'wctype_l.c',
       ]],
       ['math', [
        '__cos.c',
        '__cosdf.c',
        '__sin.c',
        '__sindf.c',
        'ilogb.c',
        'ilogbf.c',
        'ilogbl.c',
        'ldexp.c',
        'ldexpf.c',
        'ldexpl.c',
        'logb.c',
        'logbf.c',
        'logbl.c',
        'lgamma.c',
        'lgamma_r.c',
        'lgammaf.c',
        'lgammaf_r.c',
        'lgammal.c',
        'scalbnf.c',
        'signgam.c',
        'tgamma.c',
        'tgammaf.c',
        'tgammal.c'
       ]],
       ['misc', [
        'getopt.c',
        'getopt_long.c',
       ]],
       ['multibyte', [
        'btowc.c',
        'internal.c',
        'mblen.c',
        'mbrlen.c',
        'mbrtowc.c',
        'mbsinit.c',
        'mbsnrtowcs.c',
        'mbsrtowcs.c',
        'mbstowcs.c',
        'mbtowc.c',
        'wcrtomb.c',
        'wcsnrtombs.c',
        'wcsrtombs.c',
        'wcstombs.c',
        'wctob.c',
        'wctomb.c',
       ]],
       ['regex', [
        'fnmatch.c',
        'regcomp.c',
        'regerror.c',
        'regexec.c',
        'tre-mem.c',
       ]],
       ['stdio', [
        'fwprintf.c',
        'swprintf.c',
        'vfwprintf.c',
        'vswprintf.c',
        'vwprintf.c',
        'wprintf.c',
        'fputwc.c',
        'fputws.c',
       ]],
       ['stdlib', [
         'ecvt.c',
         'fcvt.c',
         'gcvt.c',
         'wcstod.c',
         'wcstol.c',
       ]],
       ['string', [
         'memccpy.c',
         'memmem.c',
         'mempcpy.c',
         'memrchr.c',
         'strcasestr.c',
         'strchrnul.c',
         'strlcat.c',
         'strlcpy.c',
         'strsep.c',
         'strverscmp.c',
         'wcpcpy.c',
         'wcpncpy.c',
         'wcscasecmp.c',
         'wcscasecmp_l.c',
         'wcscat.c',
         'wcschr.c',
         'wcscmp.c',
         'wcscpy.c',
         'wcscspn.c',
         'wcsdup.c',
         'wcslen.c',
         'wcsncasecmp.c',
         'wcsncasecmp_l.c',
         'wcsncat.c',
         'wcsncmp.c',
         'wcsncpy.c',
         'wcsnlen.c',
         'wcspbrk.c',
         'wcsrchr.c',
         'wcsspn.c',
         'wcsstr.c',
         'wcstok.c',
         'wcswcs.c',
         'wmemchr.c',
         'wmemcmp.c',
         'wmemcpy.c',
         'wmemmove.c',
         'wmemset.c',
       ]]
    ]
    libcextra_files = []
    for directory, sources in musl_files:
      libcextra_files += [os.path.join('libc', 'musl', 'src', directory, source) for source in sources]
    return build_libc('libcextra.bc', libcextra_files)

  # libcxx
  def create_libcxx():
    logging.debug('building libcxx for cache')
    libcxx_files = [
      'algorithm.cpp',
      'condition_variable.cpp',
      'future.cpp',
      'iostream.cpp',
      'memory.cpp',
      'random.cpp',
      'stdexcept.cpp',
      'system_error.cpp',
      'utility.cpp',
      'bind.cpp',
      'debug.cpp',
      'hash.cpp',
      'mutex.cpp',
      'string.cpp',
      'thread.cpp',
      'valarray.cpp',
      'chrono.cpp',
      'exception.cpp',
      'ios.cpp',
      'locale.cpp',
      'regex.cpp',
      'strstream.cpp'
    ]
    return build_libcxx(os.path.join('system', 'lib', 'libcxx'), 'libcxx.bc', libcxx_files)

  def apply_libcxx(need):
    assert shared.Settings.QUANTUM_SIZE == 4, 'We do not support libc++ with QUANTUM_SIZE == 1'
    # libcxx might need corrections, so turn them all on. TODO: check which are actually needed
    shared.Settings.CORRECT_SIGNS = shared.Settings.CORRECT_OVERFLOWS = shared.Settings.CORRECT_ROUNDINGS = 1
    #logging.info('using libcxx turns on CORRECT_* options')
    return True

  # libcxxabi - just for dynamic_cast for now
  def create_libcxxabi():
    logging.debug('building libcxxabi for cache')
    libcxxabi_files = [
      'typeinfo.cpp',
      'private_typeinfo.cpp'
    ]
    return build_libcxx(os.path.join('system', 'lib', 'libcxxabi', 'src'), 'libcxxabi.bc', libcxxabi_files)

  def apply_libcxxabi(need):
    assert shared.Settings.QUANTUM_SIZE == 4, 'We do not support libc++abi with QUANTUM_SIZE == 1'
    #logging.info('using libcxxabi, this may need CORRECT_* options')
    #shared.Settings.CORRECT_SIGNS = shared.Settings.CORRECT_OVERFLOWS = shared.Settings.CORRECT_ROUNDINGS = 1
    return True

  # gl
  def create_gl():
    prev_cxx = os.environ.get('EMMAKEN_CXX')
    if prev_cxx: os.environ['EMMAKEN_CXX'] = ''
    o = in_temp('gl.o')
    execute([shared.PYTHON, shared.EMCC, shared.path_from_root('system', 'lib', 'gl.c'), '-o', o])
    if prev_cxx: os.environ['EMMAKEN_CXX'] = prev_cxx
    return o

  # Settings this in the environment will avoid checking dependencies and make building big projects a little faster
  # 1 means include everything; otherwise it can be the name of a lib (libcxx, etc.)
  force = os.environ.get('EMCC_FORCE_STDLIBS')
  force_all = force == '1'

  # Scan symbols
  symbolses = map(lambda temp_file: shared.Building.llvm_nm(temp_file), temp_files)

  # Add in some hacks for js libraries. If a js lib depends on a symbol provided by a C library, it must be
  # added to here, because our deps go only one way (each library here is checked, then we check the next
  # in order - libcxx, libcxextra, etc. - and then we run the JS compiler and provide extra symbols from
  # library*.js files. But we cannot then go back to the C libraries if a new dep was added!
  # TODO: Move all __deps from src/library*.js to deps_info.json, and use that single source of info
  #       both here and in the JS compiler.
  deps_info = json.loads(open(shared.path_from_root('src', 'deps_info.json')).read())
  def add_back_deps(need):
    for ident, deps in deps_info.iteritems():
      if ident in need.undefs:
        for dep in deps:
          need.undefs.add(dep)
          shared.Settings.EXPORTED_FUNCTIONS.append('_' + dep)
  for symbols in symbolses:
    add_back_deps(symbols)

  all_needed = set()
  for symbols in symbolses:
    all_needed.update(symbols.undefs)
  for symbols in symbolses:
    all_needed.difference_update(symbols.defs)

  # Go over libraries to figure out which we must include
  # If we have libcxx, we must force inclusion of libc, since libcxx uses new internally. Note: this is kind of hacky.
  ret = []
  has = need = None
  for name, create, apply_, library_symbols in [('libcxx',    create_libcxx,    apply_libcxx,    libcxx_symbols),
                                                ('libcextra', create_libcextra, lambda x: True,  libcextra_symbols),
                                                ('libcxxabi', create_libcxxabi, apply_libcxxabi, libcxxabi_symbols),
                                                ('gl',        create_gl,        lambda x: True,  gl_symbols),
                                                ('libc',      create_libc,      apply_libc,      libc_symbols)]:
    force_this = force_all or force == name
    if not force_this:
      need = set()
      has = set()
      for symbols in symbolses:
        for library_symbol in library_symbols:
          if library_symbol in symbols.undefs:
            need.add(library_symbol)
          if library_symbol in symbols.defs:
            has.add(library_symbol)
      for haz in has: # remove symbols that are supplied by another of the inputs
        if haz in need:
          need.remove(haz)
      if shared.Settings.VERBOSE: logging.debug('considering %s: we need %s and have %s' % (name, str(need), str(has)))
    if force_this or len(need) > 0:
      force_all = True
      if apply_(need):
        # We need to build and link the library in
        logging.debug('including %s' % name)
        libfile = shared.Cache.get(name, create)
        ret.append(libfile)
  return ret

