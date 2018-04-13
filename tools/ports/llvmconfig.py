import os
import logging
from subprocess import PIPE

TAG = 'incoming'
PORT_NAME = 'llvm-config'
PORT_KEY = 'emscripten-fastcomp-' + TAG

def getroot(ports):
    return os.path.join(ports.get_dir(), PORT_NAME, PORT_KEY)

def get(ports, settings, shared):
    ports.fetch_project(PORT_NAME, 'https://github.com/kripken/emscripten-fastcomp/archive/' + TAG + '.zip', PORT_KEY)

    def create():
        logging.info('building: %s' % PORT_KEY)

        root = getroot(ports)
        buildroot = os.path.join(root, 'build')

        if not(os.path.exists(buildroot)):
            os.mkdir(buildroot)

        ports.build_native(buildroot, '..', target='llvm-config')
        ports.build_native(buildroot, '..', target='intrinsics_gen')

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
            # http://clang-developers.42468.n3.nabble.com/How-to-determine-clang-s-system-include-dirs-to-set-in-ASTVisitor-td4029080.html
            foo = os.path.join(shared.get_emscripten_temp_dir(), 'foo.cpp')
            with open(foo, 'w') as f:
                pass

            out = shared.run_process([shared.CLANG_CPP, '-###', '-c', foo], stderr=PIPE).stderr
            print("========\/")
            print(out)
            print("===")

            idx = 0
            tokens = out.split()
            for next in tokens:
                idx += 1
                if '-resource-dir' in next:
                    break

            assert(idx < len(tokens))
            compile_flags += ['-I' + os.path.join(tokens[idx].replace("'", "").replace('"', ''), 'include')]
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