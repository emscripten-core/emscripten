import os
import logging
from subprocess import PIPE
from shutil import copyfile

TAG = 'incoming'

PORT_NAME = 'llvm-config'
PORT_KEY = PORT_NAME + '-' + TAG

FASTCOMP_PORT = 'emscripten-fastcomp'
FASTCOMP_KEY = FASTCOMP_PORT + '-' + TAG

FASTCOMP_CLANG_PORT = 'emscripten-fastcomp-clang'
FASTCOMP_CLANG_KEY = FASTCOMP_CLANG_PORT + '-' + TAG

def getroot(ports):
    return os.path.join(ports.get_dir(), PORT_NAME)

def should_build_llvm_config(settings):
    return len(find_cpp_passes(settings)) > 0

def find_cpp_passes(settings):
    cpp_passes = []

    if not hasattr(settings, 'EXTRA_LLVM_FINAL_OPT_ARGS'):
        return cpp_passes

    for idx, val in enumerate(settings.EXTRA_LLVM_FINAL_OPT_ARGS):
        if val.strip() == '-load':
            fileindex = idx + 1
            loadfile = settings.EXTRA_LLVM_FINAL_OPT_ARGS[fileindex]
            if loadfile.endswith('.cpp'):
                cpp_passes += [fileindex]

    return cpp_passes

def get(ports, settings, shared):
    if not should_build_llvm_config(settings):
        return []

    ports.fetch_project(FASTCOMP_PORT, 'https://github.com/kripken/emscripten-fastcomp/archive/' + TAG + '.zip', FASTCOMP_KEY)
    ports.fetch_project(FASTCOMP_CLANG_PORT, 'https://github.com/kripken/emscripten-fastcomp-clang/archive/' + TAG + '.zip', FASTCOMP_CLANG_KEY)

    def create():
        logging.info('building: %s' % (PORT_NAME + '-' + TAG))

        root = getroot(ports)
        buildroot = os.path.join(root, 'build')
        includeroot = os.path.join(root, 'include')

        shared.safe_ensure_dirs(buildroot)
        shared.safe_ensure_dirs(includeroot)

        ports.build_native(buildroot, os.path.join(ports.get_dir(), FASTCOMP_PORT, FASTCOMP_KEY), target='llvm-config')
        ports.build_native(buildroot, os.path.join(ports.get_dir(), FASTCOMP_PORT, FASTCOMP_KEY), target='intrinsics_gen')

        # copyfile(os.path.join(ports.get_dir(), FASTCOMP_CLANG_PORT, FASTCOMP_CLANG_KEY, 'lib', 'Headers', 'stdarg.h'),
        #          os.path.join(includeroot, 'stdarg.h'))

        # the "output" of this port build is a tag file, saying which port we have
        tag_file = os.path.join(root, 'tag.txt')
        open(tag_file, 'w').write(TAG)
        return tag_file

    return [shared.Cache.get(PORT_KEY, create, what='port', extension='.txt')]


def compile_pass(src, compile_flags, shared):
    def lib_suffix(name):
        return name + '.dll' if shared.WINDOWS else name + '.so'

    temp_dir = shared.get_emscripten_temp_dir()
    so = os.path.join(temp_dir, lib_suffix(os.path.basename(src)))

    logging.info("@@@DEBUG")
    logging.info("Building %s -> %s" % (src, so))

    logging.info("Building %s -> %s" % (src, so))

    cmd = [shared.CLANG_CPP]
    cmd += compile_flags
    cmd += ['-shared', src, '-o', so]

    logging.info("Run %s" % cmd)
    out = shared.run_process(cmd, stdout=PIPE, stderr=PIPE, check=False)
    logging.info("== STDOUT:\n%s\n== STDERR:\n%s\n" % (out.stdout, out.stderr))
    assert(out.returncode == 0)

    return so

def compile_flags(ports, shared):
    compile_flags = []
    binroot = os.path.join(getroot(ports), 'build', 'bin')

    # llvm-config flags
    llvmconfig = os.path.join(binroot, shared.exe_suffix('llvm-config'))
    cmd = [llvmconfig, '--cxxflags']
    flags = shared.run_process(cmd, check=True, stdout=PIPE).stdout
    compile_flags += flags.split()
    compile_flags += ['-D_GLIBCXX_USE_CXX11_ABI=0']  # https://stackoverflow.com/questions/37366291/undefined-symbol-for-self-built-llvm-opt
    compile_flags += ['-fno-rtti'] # https://stackoverflow.com/questions/42998440/undefined-symbol-for-llvm-plugin

    # clang includes
    # compile_flags += ['-I' + os.path.join(getroot(ports), 'include')]
    compile_flags += ['-I' + os.path.join(ports.get_dir(), FASTCOMP_CLANG_PORT, FASTCOMP_CLANG_KEY, 'lib', 'Headers')]

    return compile_flags

def process_dependencies(settings):
    pass


def process_args(ports, args, settings, shared):
    flags = []
    cpp_passes = find_cpp_passes(settings)

    if len(cpp_passes) > 0:
        get(ports, settings, shared)
        flags = compile_flags(ports, shared)

    for idx in cpp_passes:
        settings.EXTRA_LLVM_FINAL_OPT_ARGS[idx] = compile_pass(settings.EXTRA_LLVM_FINAL_OPT_ARGS[idx], flags, shared)

    return args


def show():
    return PORT_NAME