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

def get(ports, settings, shared):
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

        copyfile(os.path.join(ports.get_dir(), FASTCOMP_CLANG_PORT, FASTCOMP_CLANG_KEY, 'lib', 'Headers', 'stdarg.h'),
                 os.path.join(includeroot, 'stdarg.h'))

        # the "output" of this port build is a tag file, saying which port we have
        tag_file = os.path.join(root, 'tag.txt')
        open(tag_file, 'w').write(TAG)
        return tag_file

    return [shared.Cache.get(PORT_KEY, create, what='port', extension='.txt')]


def process_dependencies(settings):
    pass


def process_args(ports, args, settings, shared):
    scope = [True, []]

    def lib_suffix(name):
        return name + '.dll' if shared.WINDOWS else name + '.so'

    def compile_shared(src):
        compile_flags = scope[1]

        temp_dir = shared.get_emscripten_temp_dir()
        so = os.path.join(temp_dir, lib_suffix(os.path.basename(src)))

        print("@@@DEBUG")
        print("Building %s -> %s" % (src, so))

        logging.info("Building %s -> %s" % (src, so))

        if len(compile_flags) == 0:
            binroot = os.path.join(getroot(ports), 'build', 'bin')

            # llvm-config flags
            llvmconfig = os.path.join(binroot, shared.exe_suffix('llvm-config'))
            cmd = [llvmconfig, '--cxxflags']
            flags = shared.run_process(cmd, check=True, stdout=PIPE).stdout
            compile_flags += flags.split()
            compile_flags += ['-D_GLIBCXX_USE_CXX11_ABI=0']  # https://stackoverflow.com/questions/37366291/undefined-symbol-for-self-built-llvm-opt)

            # clang includes
            compile_flags += ['-I' + os.path.join(getroot(ports), 'include')]
            scope[1] = compile_flags

        cmd = [shared.CLANG_CPP]
        cmd += compile_flags
        cmd += ['-shared', src, '-o', so]

        print("Run", cmd)
        out = shared.run_process(cmd, stdout=PIPE, stderr=PIPE, check=False)
        print("== STDOUT:\n%s\n== STDERR:\n%s\n" % (out.stdout, out.stderr))
        assert(out.returncode == 0)

        return so

    def getport():
        if scope[0]:
            get(ports, settings, shared)
            scope[0] = False

    idx = 0
    while idx < len(settings.EXTRA_LLVM_FINAL_OPT_ARGS):
        if settings.EXTRA_LLVM_FINAL_OPT_ARGS[idx].strip() == '-load':
            idx += 1
            loadfile = settings.EXTRA_LLVM_FINAL_OPT_ARGS[idx]

            if loadfile.endswith('.cpp'):
                getport()
                so = compile_shared(loadfile)
                settings.EXTRA_LLVM_FINAL_OPT_ARGS[idx] = so
        idx += 1

    return args


def show():
    return PORT_NAME