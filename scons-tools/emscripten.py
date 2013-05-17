import hashlib
import json
import sys
import os
from SCons.Defaults import Delete
from SCons.Builder import Builder
from SCons.Scanner import Scanner

def exists(env):
    return True

def _expand_settings_flags(settings, env):
    return [
        ('-s%s=%s' % (KEY, json.dumps(VALUE).replace('"', '\\"')))
        for KEY, VALUE in settings.items() ]

emscripten_version_files = {}

def build_version_file(env):
    if not env.subst('$EMSCRIPTEN_VERSION_FILE'):
        raise AssertionError('Must set EMSCRIPTEN_VERSION_FILE in environment')
    if not env.subst('$EMSCRIPTEN_TEMP_DIR'):
        raise AssertionError('Must set EMSCRIPTEN_TEMP_DIR in environment')
    
    EMSCRIPTEN_DEPENDENCIES = [
        env.Glob('${EMSCRIPTEN_HOME}/src/*.js'),
        env.Glob('${EMSCRIPTEN_HOME}/tools/*.py'),
        '${EMSCRIPTEN_HOME}/emscripten.py',
    ]
    if env.subst('$EMSCRIPTEN_SHELL'):
        EMSCRIPTEN_DEPENDENCIES.append('$EMSCRIPTEN_SHELL')

    def touch_file(target, source, env):
        m = hashlib.md5()
        for s in source:
            m.update(file(s.abspath, 'rb').read())
        for t in target:
            file(t.abspath, 'wb').write(m.hexdigest())

    [emscripten_version_file] = env.Command(
        '$EMSCRIPTEN_VERSION_FILE',
        EMSCRIPTEN_DEPENDENCIES,
        touch_file)

    env.AddPostAction(
        emscripten_version_file,
        Delete(env.Dir('$EMSCRIPTEN_TEMP_DIR').abspath))

    return emscripten_version_file

def get_emscripten_version_file(env):
    EMSCRIPTEN_HOME = env.Dir('$EMSCRIPTEN_HOME').abspath
    try:
        version_file = emscripten_version_files[EMSCRIPTEN_HOME]
    except KeyError:
        version_file = build_version_file(env)
        emscripten_version_files[EMSCRIPTEN_HOME] = version_file
    return version_file

def depend_on_emscripten(node, env, path):
    return [get_emscripten_version_file(env)]

EmscriptenScanner = Scanner(
    name='emscripten',
    function=depend_on_emscripten)

def setExtension(filename, extension):
    return os.path.splitext(filename)[0] + '.' + extension

def emscripten(env, target_js, source_bc):
    env = env.Clone()
    def buildName(extension):
        return setExtension(target_js, extension)

    # for debugging and reading generated code.
    # not in critical path, uses spare cores.
    env.LLVMDis(buildName('ll'), source_bc)

    [opt_ll] = env.LLVMOpt(
        buildName('opt.ll'),
        source_bc,
        LLVM_OPT_FLAGS=['-S'])

    [raw_emscripten_js] = env.Emscripten(
        buildName('raw.js'),
        [opt_ll])

    [optimized_js] = env.JSOptimizer(
        buildName('opt.js'),
        raw_emscripten_js)

    prejs = [
        env['EMSCRIPTEN_PREJS'],
        '${EMSCRIPTEN_HOME}/src/embind/emval.js',
        '${EMSCRIPTEN_HOME}/src/embind/embind.js' ]

    [concatenated_js] = env.Concatenate(
        buildName('concat.js'),
        [ prejs,
          optimized_js,
          env['EMSCRIPTEN_POSTJS'] ])

    DISABLE_EMSCRIPTEN_WARNINGS = [
        '--jscomp_error', 'ambiguousFunctionDecl',
        '--jscomp_error', 'checkDebuggerStatement',
        '--jscomp_off', 'checkTypes',
        '--jscomp_off', 'checkVars',
        '--jscomp_error', 'deprecated',
        '--jscomp_off', 'duplicate',
        #'--jscomp_error', 'es5strict',
        '--jscomp_off', 'missingProperties', # TODO: fix emscripten and turn this one on
        '--jscomp_error', 'undefinedNames',
        '--jscomp_off', 'undefinedVars', # TODO: fix emscripten and turn this one on
        '--jscomp_off', 'uselessCode',
        '--jscomp_off', 'globalThis',
    ]

    [iter_global_emscripten_js] = env.Concatenate(
        buildName('iter.js'),
        [ prejs,
          raw_emscripten_js,
          env['EMSCRIPTEN_POSTJS'] ])

    [global_cc_emscripten_js] = env.ClosureCompiler(
        buildName('global.closure.js'),
        concatenated_js,
        CLOSURE_FLAGS=['--language_in', 'ECMASCRIPT5']+DISABLE_EMSCRIPTEN_WARNINGS+['--formatting', 'PRETTY_PRINT', '--compilation_level', 'SIMPLE_OPTIMIZATIONS'])

    #env.Append(
    #    NODEJSFLAGS=['--max-stack-size=1000000000'],
    #    UGLIFYJSFLAGS=['--stats', '-c', 'warnings=false', '-b'])
    #env.UglifyJS(
    #    buildName('global.uglify.js'),
    #    concatenated_js)

    [closure_js] = env.ClosureCompiler(
        buildName('closure.js'),
        concatenated_js,
        CLOSURE_FLAGS=['--language_in', 'ECMASCRIPT5']+DISABLE_EMSCRIPTEN_WARNINGS+['--formatting', 'PRETTY_PRINT', '--compilation_level', 'ADVANCED_OPTIMIZATIONS'])

    [global_emscripten_min_js] = env.JSOptimizer(
        buildName('global.min.js'),
        closure_js,
        JS_OPTIMIZER_PASSES=['simplifyExpressionsPost', 'compress', 'last'])

    [emscripten_iteration_js] = env.WrapInModule(
        buildName('iteration.js'),
        iter_global_emscripten_js)

    [emscripten_js] = env.WrapInModule(
        buildName('debug.js'),
        global_cc_emscripten_js)

    [emscripten_min_js] = env.WrapInModule(
        buildName('min.js'),
        global_emscripten_min_js)

    env.InstallAs(buildName('js'), emscripten_js)

    return [emscripten_iteration_js, emscripten_js, emscripten_min_js]

LIBC_SOURCES = [
    'system/lib/dlmalloc.c',
    'system/lib/libc/musl/src/string/wmemset.c',
    'system/lib/libc/musl/src/string/wmemcpy.c',
]

LIBCXX_SOURCES = [os.path.join('system/lib/libcxx', x) for x in [
    'algorithm.cpp',
    'bind.cpp',
    #'chrono.cpp',
    #'condition_variable.cpp',
    #'debug.cpp',
    #'exception.cpp',
    'future.cpp',
    'hash.cpp',
    #'ios.cpp',
    #'iostream.cpp',
    'memory.cpp',
    'mutex.cpp',
    'new.cpp',
    'random.cpp',
    'regex.cpp',
    'stdexcept.cpp',
    'string.cpp',
    'strstream.cpp',
    'system_error.cpp',
    #'thread.cpp',
    'typeinfo.cpp',
    'utility.cpp',
    'valarray.cpp',
]]

LIBCXXABI_SOURCES = [os.path.join('system/lib/libcxxabi/src', x) for x in [
    'private_typeinfo.cpp'
]]

# MAJOR HACK ALERT
# ugh, SCons imports tool .py files multiple times, meaning that global variables aren't really global
# store our "globals" "privately" on the SCons object :(
import SCons

def build_libembind(env):
    emscripten_temp_dir = env.Dir('$EMSCRIPTEN_TEMP_DIR').abspath
    try:
        libembind_cache = SCons.__emscripten_libembind_cache
    except AttributeError:
        libembind_cache = {}
        SCons.__emscripten_libembind_cache = libembind_cache
    try:
        return libembind_cache[emscripten_temp_dir]
    except KeyError:
        pass

    libembind = env.Object(
        '$EMSCRIPTEN_TEMP_DIR/internal_libs/bind',
        '$EMSCRIPTEN_HOME/system/lib/embind/bind.cpp')
    env.Depends(libembind, get_emscripten_version_file(env))
    libembind_cache[emscripten_temp_dir] = libembind
    return libembind

def build_libcxx(env):
    emscripten_temp_dir = env.Dir('$EMSCRIPTEN_TEMP_DIR').abspath
    try:
        libcxx_cache = SCons.__emscripten_libcxx_cache
    except AttributeError:
        libcxx_cache = {}
        SCons.__emscripten_libcxx_cache = libcxx_cache
    try:
        return libcxx_cache[emscripten_temp_dir]
    except KeyError:
        pass

    env = env.Clone()
    env['CXXFLAGS'] = filter(lambda e: e not in ('-Werror', '-Wall'), env['CXXFLAGS'])
    env['CCFLAGS'] = filter(lambda e: e not in ('-Werror', '-Wall'), env['CCFLAGS'])

    objs = [
        env.Object(
            '${EMSCRIPTEN_TEMP_DIR}/libcxx_objects/' + os.path.splitext(o)[0] + '.bc',
            '${EMSCRIPTEN_HOME}/' + o)
        for o in LIBC_SOURCES + LIBCXXABI_SOURCES + LIBCXX_SOURCES]
    env.Depends(objs, get_emscripten_version_file(env))

    libcxx = env.Library('${EMSCRIPTEN_TEMP_DIR}/internal_libs/libcxx', objs)
    libcxx_cache[emscripten_temp_dir] = libcxx
    return libcxx

def generate(env):
    env.SetDefault(
        PYTHON=sys.executable,
        NODEJS='node',
        JS_ENGINE='$NODEJS',
        EMSCRIPTEN_FLAGS=['-v', '-j', '--suppressUsageWarning'],
        EMSCRIPTEN_TEMP_DIR=env.Dir('#/emscripten.tmp'),
        _expand_settings_flags=_expand_settings_flags,
        EMSCRIPTEN_PREJS=[],
        EMSCRIPTEN_POSTJS=[],
        EMSCRIPTEN_SETTINGS={},
        _EMSCRIPTEN_SETTINGS_FLAGS='${_expand_settings_flags(EMSCRIPTEN_SETTINGS, __env__)}',
        JS_OPTIMIZER_PASSES=[],
        LLVM_OPT_PASSES=['-std-compile-opts', '-std-link-opts'],

        EMSCRIPTEN_HOME=env.Dir(os.path.join(os.path.dirname(__file__), '..')),
    )

    env.Replace(
        CC='${LLVM_ROOT}/${CLANG}',
        CXX='${LLVM_ROOT}/${CLANGXX}',
        AR='${LLVM_ROOT}/${LLVM_LINK}',
        ARCOM='$AR -o $TARGET $SOURCES',
        OBJSUFFIX='.bc',
        LIBPREFIX='',
        LIBSUFFIX='.bc',
        RANLIBCOM='',
        CCFLAGS=[
            '-U__STRICT_ANSI__',
            '-target', 'le32-unknown-nacl',
            '-nostdinc',
            '-Wno-#warnings',
            '-Wno-error=unused-variable',
            '-Werror',
            '-Os',
            '-fno-threadsafe-statics',
            '-fvisibility=hidden',
            '-fvisibility-inlines-hidden',
            '-Xclang', '-nostdinc++',
            '-Xclang', '-nobuiltininc',
            '-Xclang', '-nostdsysteminc',
            '-Xclang', '-isystem$EMSCRIPTEN_HOME/system/include',
            '-Xclang', '-isystem$EMSCRIPTEN_HOME/system/include/libc',
            '-Xclang', '-isystem$EMSCRIPTEN_HOME/system/include/libcxx',
            '-Xclang', '-isystem$EMSCRIPTEN_HOME/system/include/bsd',
            '-emit-llvm'],
        CXXFLAGS=['-std=c++11', '-fno-exceptions'],
    )
    env.Append(CPPDEFINES=[
        'EMSCRIPTEN',
        '__EMSCRIPTEN__',
        '__STDC__',
        '__IEEE_LITTLE_ENDIAN',
    ])
    
    env['BUILDERS']['Emscripten'] = Builder(
        action='$PYTHON ${EMSCRIPTEN_HOME}/emscripten.py $EMSCRIPTEN_FLAGS $_EMSCRIPTEN_SETTINGS_FLAGS --temp-dir=$EMSCRIPTEN_TEMP_DIR --compiler $JS_ENGINE --relooper=third-party/relooper.js $SOURCE > $TARGET',
        target_scanner=EmscriptenScanner)

    env['BUILDERS']['JSOptimizer'] = Builder(
        action='$JS_ENGINE ${EMSCRIPTEN_HOME}/tools/js-optimizer.js $SOURCE $JS_OPTIMIZER_PASSES > $TARGET',
        target_scanner=EmscriptenScanner)

    def depend_on_embedder(target, source, env):
        env.Depends(target, env['JS_EMBEDDER'])
        return target, source

    def embed_files_in_js(target, source, env, for_signature):
        return '$PYTHON $JS_EMBEDDER $SOURCE.srcpath > $TARGET'
    
    def get_files_in_tree(node, env, path):
        tree_paths = []
        for root, dirs, files in os.walk(str(node)):
            tree_paths += [os.path.join(root, f) for f in files]
        return [env.File(p) for p in tree_paths]

    env.SetDefault(
        JS_EMBEDDER=env.File('#/bin/embed_files_in_js.py'))
    
    FileTreeScanner = Scanner(
        function=get_files_in_tree,
        name='FileTreeScanner',
        recursive=False)

    env['BUILDERS']['EmbedFilesInJS'] = Builder(
        generator=embed_files_in_js,
        emitter=depend_on_embedder,
        source_scanner=FileTreeScanner)

    env.AddMethod(emscripten)
    
    def ConcatenateAction(target, source, env):
        [target] = target
        total = ''.join(file(str(s), 'rb').read() for s in source)
        file(str(target), 'wb').write(total)
    env['BUILDERS']['Concatenate'] = Builder(action=ConcatenateAction)

    libembind = build_libembind(env)
    libcxx = build_libcxx(env)

    # should embind be its own tool?
    env.Append(
        CPPPATH=[
            '${EMSCRIPTEN_HOME}/system/include' ],
        LIBPATH=['$EMSCRIPTEN_TEMP_DIR/internal_libs'],
        LIBS=[
            libembind,
            libcxx,
        ],
    )

