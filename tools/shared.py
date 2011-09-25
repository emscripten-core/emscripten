import shutil, time, os
from subprocess import Popen, PIPE, STDOUT

abspath = os.path.abspath(os.path.dirname(__file__))
def path_from_root(*pathelems):
  return os.path.join(os.path.sep, *(abspath.split(os.sep)[:-1] + list(pathelems)))

CONFIG_FILE = os.path.expanduser('~/.emscripten')
if not os.path.exists(CONFIG_FILE):
  shutil.copy(path_from_root('settings.py'), CONFIG_FILE)
exec(open(CONFIG_FILE, 'r').read())

# Tools

CLANG=os.path.expanduser(os.path.join(LLVM_ROOT, 'clang++'))
LLVM_LINK=os.path.join(LLVM_ROOT, 'llvm-link')
LLVM_LD=os.path.join(LLVM_ROOT, 'llvm-ld')
LLVM_OPT=os.path.expanduser(os.path.join(LLVM_ROOT, 'opt'))
LLVM_AS=os.path.expanduser(os.path.join(LLVM_ROOT, 'llvm-as'))
LLVM_DIS=os.path.expanduser(os.path.join(LLVM_ROOT, 'llvm-dis'))
LLVM_DIS_OPTS = ['-show-annotations'] # For LLVM 2.8+. For 2.7, you may need to do just    []
LLVM_INTERPRETER=os.path.expanduser(os.path.join(LLVM_ROOT, 'lli'))
LLVM_COMPILER=os.path.expanduser(os.path.join(LLVM_ROOT, 'llc'))

BINDINGS_GENERATOR = path_from_root('tools', 'bindings_generator.py')
EXEC_LLVM = path_from_root('tools', 'exec_llvm.py')

# Additional compiler options

USE_EMSDK = True

COMPILER_OPTS = COMPILER_OPTS + ['-m32', '-v', '-U__i386__', '-U__x86_64__', '-U__i386', '-U__x86_64', '-U__SSE__', '-U__SSE2__', '-UX87_DOUBLE_ROUNDING', '-UHAVE_GCC_ASM_FOR_X87']

if USE_EMSDK:
  COMPILER_OPTS += [ '-nostdinc',
  '-I' + path_from_root('system', 'include'),
  '-I' + path_from_root('system', 'include', 'freebsd'), # posix stuff
  '-I' + path_from_root('system', 'include', 'libc'),
  '-I' + path_from_root('system', 'include', 'libcxx'),
]

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

def run_js(engine, filename, args, check_timeout=False, stdout=PIPE, stderr=STDOUT, cwd=None):
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

def limit_size(string, MAX=800*20):
  if len(string) < MAX: return string
  return string[0:MAX/2] + '\n[..]\n' + string[-MAX/2:]

def pick_llvm_opts(optimization_level, optimize_size, allow_nonportable=False, quantum_size=4, use_aa=False):
  opts = []
  if optimization_level > 0:
    if allow_nonportable:
      opts.append('-O%d' % optimization_level)
    else:
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
      if quantum_size == 4: opts.append('-indvars') # XXX this infinite-loops raytrace on q1 (loop in |new node_t[count]| has 68 hardcoded &not fixed)
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

  return opts

def read_auto_optimize_data(filename):
  '''
    Reads the output of AUTO_OPTIMIZE and generates proper information for CORRECT_* == 2 's *_LINES options
  '''
  signs_lines = []
  overflows_lines = []
  
  for line in open(filename, 'r'):
    if line.rstrip() == '': continue
    if '%0 failures' in line: continue
    left, right = line.split(' : ')
    signature = left.split('|')[1]
    if 'Sign' in left:
      signs_lines.append(signature)
    elif 'Overflow' in left:
      overflows_lines.append(signature)

  return {
    'signs_lines': signs_lines,
    'overflows_lines': overflows_lines
  }

