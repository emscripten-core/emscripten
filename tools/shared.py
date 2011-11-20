import shutil, time, os, json
from subprocess import Popen, PIPE, STDOUT

__rootpath__ = os.path.abspath(os.path.dirname(os.path.dirname(__file__)))
def path_from_root(*pathelems):
  return os.path.join(__rootpath__, *pathelems)

CONFIG_FILE = os.path.expanduser('~/.emscripten')
if not os.path.exists(CONFIG_FILE):
  shutil.copy(path_from_root('settings.py'), CONFIG_FILE)
exec(open(CONFIG_FILE, 'r').read())

# Tools/paths

CLANG=os.path.expanduser(os.path.join(LLVM_ROOT, 'clang++'))
LLVM_LINK=os.path.join(LLVM_ROOT, 'llvm-link')
LLVM_LD=os.path.join(LLVM_ROOT, 'llvm-ld')
LLVM_OPT=os.path.expanduser(os.path.join(LLVM_ROOT, 'opt'))
LLVM_AS=os.path.expanduser(os.path.join(LLVM_ROOT, 'llvm-as'))
LLVM_DIS=os.path.expanduser(os.path.join(LLVM_ROOT, 'llvm-dis'))
LLVM_DIS_OPTS = ['-show-annotations'] # For LLVM 2.8+. For 2.7, you may need to do just    []
LLVM_INTERPRETER=os.path.expanduser(os.path.join(LLVM_ROOT, 'lli'))
LLVM_COMPILER=os.path.expanduser(os.path.join(LLVM_ROOT, 'llc'))
COFFEESCRIPT = path_from_root('tools', 'eliminator', 'node_modules', 'coffee-script', 'bin', 'coffee')

EMSCRIPTEN = path_from_root('emscripten.py')
DEMANGLER = path_from_root('third_party', 'demangler.py')
NAMESPACER = path_from_root('tools', 'namespacer.py')
EMMAKEN = path_from_root('tools', 'emmaken.py')
AUTODEBUGGER = path_from_root('tools', 'autodebugger.py')
DFE = path_from_root('tools', 'dead_function_eliminator.py')
BINDINGS_GENERATOR = path_from_root('tools', 'bindings_generator.py')
EXEC_LLVM = path_from_root('tools', 'exec_llvm.py')
VARIABLE_ELIMINATOR = path_from_root('tools', 'eliminator', 'eliminator.coffee')
JS_OPTIMIZER = path_from_root('tools', 'js-optimizer.js')

# Additional compiler options

COMPILER_OPTS = COMPILER_OPTS + ['-m32', '-U__i386__', '-U__x86_64__', '-U__i386', '-U__x86_64', '-U__SSE__', '-U__SSE2__', '-U__MMX__',
                                 '-UX87_DOUBLE_ROUNDING', '-UHAVE_GCC_ASM_FOR_X87', '-DEMSCRIPTEN', '-U__STRICT_ANSI__']

USE_EMSDK = not os.environ.get('EMMAKEN_NO_SDK')

if USE_EMSDK:
  EMSDK_OPTS = [ '-nostdinc',
  '-I' + path_from_root('system', 'include'),
  '-I' + path_from_root('system', 'include', 'bsd'), # posix stuff
  '-I' + path_from_root('system', 'include', 'libc'),
  '-I' + path_from_root('system', 'include', 'libcxx'),
  '-I' + path_from_root('system', 'include', 'gfx'),
  '-I' + path_from_root('system', 'include', 'net'),
  '-I' + path_from_root('system', 'include', 'SDL'),
] + [
  '-U__APPLE__'
]
  COMPILER_OPTS += EMSDK_OPTS

# Engine tweaks

#if 'strict' not in str(SPIDERMONKEY_ENGINE): # XXX temporarily disable strict mode until we sort out some stuff
#  SPIDERMONKEY_ENGINE += ['-e', "options('strict')"] # Strict mode in SpiderMonkey. With V8 we check that fallback to non-strict works too

if 'gcparam' not in str(SPIDERMONKEY_ENGINE):
  SPIDERMONKEY_ENGINE += ['-e', "gcparam('maxBytes', 1024*1024*1024);"] # Our very large files need lots of gc heap

# Utilities

def timeout_run(proc, timeout, note):
  start = time.time()
  if timeout is not None:
    while time.time() - start < timeout and proc.poll() is None:
      time.sleep(0.1)
    if proc.poll() is None:
      proc.kill() # XXX bug: killing emscripten.py does not kill it's child process!
      raise Exception("Timed out: " + note)
  return proc.communicate()[0]

def run_js(engine, filename, args=[], check_timeout=False, stdout=PIPE, stderr=None, cwd=None):
  return timeout_run(Popen(engine + [filename] + (['--'] if 'd8' in engine[0] else []) + args,
                     stdout=stdout, stderr=stderr, cwd=cwd), 15*60 if check_timeout else None, 'Execution')

def to_cc(cxx):
  # By default, LLVM_GCC and CLANG are really the C++ versions. This gets an explicit C version
  return cxx.replace('clang++', 'clang').replace('g++', 'gcc')

def line_splitter(data):
  """Silly little tool to split JSON arrays over many lines."""

  out = ''
  counter = 0

  for i in range(len(data)):
    out += data[i]
    if data[i] == ' ' and counter > 60:
      out += '\n'
      counter = 0
    else:
      counter += 1

  return out

def limit_size(string, MAX=80*20):
  if len(string) < MAX: return string
  return string[0:MAX/2] + '\n[..]\n' + string[-MAX/2:]

def read_pgo_data(filename):
  '''
    Reads the output of PGO and generates proper information for CORRECT_* == 2 's *_LINES options
  '''
  signs_lines = []
  overflows_lines = []
  
  for line in open(filename, 'r'):
    try:
      if line.rstrip() == '': continue
      if '%0 failures' in line: continue
      left, right = line.split(' : ')
      signature = left.split('|')[1]
      if 'Sign' in left:
        signs_lines.append(signature)
      elif 'Overflow' in left:
        overflows_lines.append(signature)
    except:
      pass

  return {
    'signs_lines': signs_lines,
    'overflows_lines': overflows_lines
  }

# Settings

class Dummy: pass

Settings = Dummy() # A global singleton. Not pretty, but nicer than passing |, settings| everywhere

# Building

class Building:
  COMPILER = CLANG
  LLVM_OPTS = False
  COMPILER_TEST_OPTS = []

  @staticmethod
  def get_building_env():
    env = os.environ.copy()
    env['RANLIB'] = env['AR'] = env['CXX'] = env['CC'] = env['LIBTOOL'] = EMMAKEN
    env['EMMAKEN_COMPILER'] = Building.COMPILER
    env['EMSCRIPTEN_TOOLS'] = path_from_root('tools')
    env['CFLAGS'] = env['EMMAKEN_CFLAGS'] = ' '.join(COMPILER_OPTS + Building.COMPILER_TEST_OPTS) # Normal CFLAGS is ignored by some configure's.
    return env

  @staticmethod
  def configure(args, stdout=None, stderr=None, env=None):
    if env is None:
      env = Building.get_building_env()
    env['EMMAKEN_JUST_CONFIGURE'] = '1'
    Popen(args, stdout=stdout, stderr=stderr, env=env).communicate()[0]
    del env['EMMAKEN_JUST_CONFIGURE']

  @staticmethod
  def make(args, stdout=None, stderr=None, env=None):
    if env is None:
      env = Building.get_building_env()
    Popen(args, stdout=stdout, stderr=stderr, env=env).communicate()[0]

  @staticmethod
  def build_library(name, build_dir, output_dir, generated_libs, configure=['./configure'], configure_args=[], make=['make'], make_args=['-j', '2'], cache=None, cache_name=None, copy_project=False, env_init={}):
    ''' Build a library into a .bc file. We build the .bc file once and cache it for all our tests. (We cache in
        memory since the test directory is destroyed and recreated for each test. Note that we cache separately
        for different compilers) '''

    if type(generated_libs) is not list: generated_libs = [generated_libs]

    temp_dir = build_dir
    if copy_project:
      project_dir = os.path.join(temp_dir, name)
      if os.path.exists(project_dir):
        shutil.rmtree(project_dir)
      shutil.copytree(path_from_root('tests', name), project_dir) # Useful in debugging sometimes to comment this out
    else:
      project_dir = build_dir
    try:
      old_dir = os.getcwd()
    except:
      old_dir = None
    os.chdir(project_dir)
    generated_libs = map(lambda lib: os.path.join(project_dir, lib), generated_libs)
    #for lib in generated_libs:
    #  try:
    #    os.unlink(lib) # make sure compilation completed successfully
    #  except:
    #    pass
    env = Building.get_building_env()
    for k, v in env_init.iteritems():
      env[k] = v
    if configure: # Useful in debugging sometimes to comment this out (and the lines below up to and including the |make| call)
      Building.configure(configure + configure_args, stdout=open(os.path.join(output_dir, 'configure_'), 'w'),
                                                     stderr=open(os.path.join(output_dir, 'configure_err'), 'w'), env=env)
    Building.make(make + make_args, stdout=open(os.path.join(output_dir, 'make_'), 'w'),
                                    stderr=open(os.path.join(output_dir, 'make_err'), 'w'), env=env)
    bc_file = os.path.join(project_dir, 'bc.bc')
    Building.link(generated_libs, bc_file)
    if cache is not None:
      cache[cache_name] = open(bc_file, 'rb').read()
    if old_dir:
      os.chdir(old_dir)
    return bc_file

  @staticmethod
  def link(files, target):
    output = Popen([LLVM_LINK] + files + ['-o', target], stdout=PIPE).communicate()[0]
    assert os.path.exists(target) and (output is None or 'Could not open input file' not in output), 'Linking error: ' + output

  # Emscripten optimizations that we run on the .ll file
  @staticmethod
  def ll_opts(filename):
    ## Remove target info. This helps LLVM opts, if we run them later
    #cleaned = filter(lambda line: not line.startswith('target datalayout = ') and not line.startswith('target triple = '),
    #                 open(filename + '.o.ll', 'r').readlines())
    #os.unlink(filename + '.o.ll')
    #open(filename + '.o.ll.orig', 'w').write(''.join(cleaned))
    shutil.move(filename + '.o.ll', filename + '.o.ll.orig')
    output = Popen(['python', DFE, filename + '.o.ll.orig', filename + '.o.ll'], stdout=PIPE).communicate()[0]
    assert os.path.exists(filename + '.o.ll'), 'Failed to run ll optimizations'

  # Optional LLVM optimizations
  @staticmethod
  def llvm_opts(filename):
    if Building.LLVM_OPTS:
      shutil.move(filename + '.o', filename + '.o.pre')
      output = Popen([LLVM_OPT, filename + '.o.pre'] + Building.LLVM_OPT_OPTS + ['-o=' + filename + '.o'], stdout=PIPE).communicate()[0]
      assert os.path.exists(filename + '.o'), 'Failed to run llvm optimizations: ' + output
      if Building.LLVM_OPTS == 2:
        print 'Unsafe LD!'
        shutil.move(filename + '.o', filename + '.o.pre')
        output = Popen([LLVM_LD, filename + '.o.pre', '-o=' + filename + '.tmp'], stdout=PIPE).communicate()[0]
        assert os.path.exists(filename + '.tmp.bc'), 'Failed to run llvm optimizations: ' + output
        shutil.move(filename + '.tmp.bc', filename + '.o')

  @staticmethod
  def llvm_dis(filename):
    # LLVM binary ==> LLVM assembly
    try:
      os.remove(filename + '.o.ll')
    except:
      pass
    output = Popen([LLVM_DIS, filename + '.o'] + LLVM_DIS_OPTS + ['-o=' + filename + '.o.ll'], stdout=PIPE).communicate()[0]
    assert os.path.exists(filename + '.o.ll'), 'Could not create .ll file: ' + output

  @staticmethod
  def llvm_as(filename):
    # LLVM assembly ==> LLVM binary
    try:
      os.remove(target)
    except:
      pass
    output = Popen([LLVM_AS, filename + '.o.ll', '-o=' + filename + '.o'], stdout=PIPE).communicate()[0]
    assert os.path.exists(filename + '.o'), 'Could not create bc file: ' + output

  @staticmethod
  def emscripten(filename, output_processor=None, append_ext=True, extra_args=[]):
    # Add some headers by default. TODO: remove manually adding these in each test
    if '-H' not in extra_args:
      extra_args += ['-H', 'libc/fcntl.h,libc/sys/unistd.h,poll.h,libc/math.h,libc/langinfo.h,libc/time.h']

    # Run Emscripten
    exported_settings = {}
    for setting in ['QUANTUM_SIZE', 'RELOOP', 'MICRO_OPTS', 'ASSERTIONS', 'USE_TYPED_ARRAYS', 'SAFE_HEAP', 'CHECK_OVERFLOWS', 'CORRECT_OVERFLOWS', 'CORRECT_SIGNS', 'CHECK_SIGNS', 'CORRECT_OVERFLOWS_LINES', 'CORRECT_SIGNS_LINES', 'CORRECT_ROUNDINGS', 'CORRECT_ROUNDINGS_LINES', 'INVOKE_RUN', 'SAFE_HEAP_LINES', 'INIT_STACK', 'PGO', 'EXPORTED_FUNCTIONS', 'EXPORTED_GLOBALS', 'BUILD_AS_SHARED_LIB', 'INCLUDE_FULL_LIBRARY', 'RUNTIME_TYPE_INFO', 'DISABLE_EXCEPTION_CATCHING', 'TOTAL_MEMORY', 'FAST_MEMORY', 'EXCEPTION_DEBUG', 'PROFILE', 'I64_MODE', 'EMULATE_UNALIGNED_ACCESSES']:
      try:
        value = eval('Settings.' + setting)
        if value is not None:
          exported_settings[setting] = value
      except:
        pass
    settings = ['-s %s=%s' % (k, json.dumps(v)) for k, v in exported_settings.items()]
    compiler_output = timeout_run(Popen(['python', EMSCRIPTEN, filename + ('.o.ll' if append_ext else ''), '-o', filename + '.o.js'] + settings + extra_args, stdout=PIPE), TIMEOUT, 'Compiling')
    #print compiler_output

    # Detect compilation crashes and errors
    if compiler_output is not None and 'Traceback' in compiler_output and 'in test_' in compiler_output: print compiler_output; assert 0
    assert os.path.exists(filename + '.o.js') and len(open(filename + '.o.js', 'r').read()) > 0, 'Emscripten failed to generate .js: ' + str(compiler_output)

    if output_processor is not None:
      output_processor(open(filename + '.o.js').read())

  @staticmethod
  def pick_llvm_opts(optimization_level, safe=True):
    '''
      It may be safe to use nonportable optimizations (like -OX) if we remove the platform info from the .ll
      (which we do in do_ll_opts) - but even there we have issues (even in TA2) with instruction combining
      into i64s. In any case, the handpicked ones here should be safe and portable. They are also tuned for
      things that look useful.
    '''
    if not Building.LLVM_OPTS: return # XXX this makes using this independently from the test runner very unpleasant

    opts = []
    if optimization_level > 0:
      if not safe:
        #opts.append('-O%d' % optimization_level)
        opts.append('-std-compile-opts')
        opts.append('-std-link-opts')
        print 'Unsafe:', opts
      else:
        allow_nonportable = not safe
        optimize_size = True
        use_aa = not safe

        # PassManagerBuilder::populateModulePassManager
        if allow_nonportable and use_aa: # ammo.js results indicate this can be nonportable
          opts.append('-tbaa')
          opts.append('-basicaa') # makes fannkuch slow but primes fast

        opts.append('-globalopt')
        opts.append('-ipsccp')
        opts.append('-deadargelim')
        if allow_nonportable: opts.append('-instcombine')
        opts.append('-simplifycfg')

        opts.append('-prune-eh')
        if not optimize_size: opts.append('-inline') # The condition here is a difference with LLVM's createStandardAliasAnalysisPasses
        opts.append('-functionattrs')
        if optimization_level > 2:
          opts.append('-argpromotion')

        # XXX Danger: Can turn a memcpy into something that violates the
        #             load-store consistency hypothesis. See hashnum() in Lua.
        #             Note: this opt is of great importance for raytrace...
        if allow_nonportable: opts.append('-scalarrepl')

        if allow_nonportable: opts.append('-early-cse') # ?
        opts.append('-simplify-libcalls')
        opts.append('-jump-threading')
        if allow_nonportable: opts.append('-correlated-propagation') # ?
        opts.append('-simplifycfg')
        if allow_nonportable: opts.append('-instcombine')

        opts.append('-tailcallelim')
        opts.append('-simplifycfg')
        opts.append('-reassociate')
        opts.append('-loop-rotate')
        opts.append('-licm')
        opts.append('-loop-unswitch') # XXX should depend on optimize_size
        if allow_nonportable: opts.append('-instcombine')
        if Settings.QUANTUM_SIZE == 4: opts.append('-indvars') # XXX this infinite-loops raytrace on q1 (loop in |new node_t[count]| has 68 hardcoded &not fixed)
        if allow_nonportable: opts.append('-loop-idiom') # ?
        opts.append('-loop-deletion')
        opts.append('-loop-unroll')

        ##### not in llvm-3.0. but have |      #addExtensionsToPM(EP_LoopOptimizerEnd, MPM);| if allow_nonportable: opts.append('-instcombine')

        # XXX Danger: Messes up Lua output for unknown reasons
        #             Note: this opt is of minor importance for raytrace...
        if optimization_level > 1 and allow_nonportable: opts.append('-gvn')

        opts.append('-memcpyopt') # Danger?
        opts.append('-sccp')

        if allow_nonportable: opts.append('-instcombine')
        opts.append('-jump-threading')
        opts.append('-correlated-propagation')
        opts.append('-dse')
        #addExtensionsToPM(EP_ScalarOptimizerLate, MPM);

        opts.append('-adce')
        opts.append('-simplifycfg')
        if allow_nonportable: opts.append('-instcombine')

        opts.append('-strip-dead-prototypes')

        if optimization_level > 2: opts.append('-globaldce')

        if optimization_level > 1: opts.append('-constmerge')

    Building.LLVM_OPT_OPTS = opts
    return opts

