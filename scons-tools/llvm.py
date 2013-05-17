from SCons.Scanner.Prog import scan
from SCons.Builder import Builder

def exists(env):
    return True

def add_libraries(target, source, env):
    unique = []
    lib_nodes = set()
    for x in scan(None, env, tuple(map(env.Dir, env['LIBPATH']))):
        if x in lib_nodes:
            continue
        lib_nodes.add(x)
        unique.append(x)
    return (target, source + unique)

def generate(env):
    env['BUILDERS']['LLVMDis'] = Builder(
        action='${LLVM_ROOT}/llvm-dis -o=$TARGET $SOURCE')

    env['BUILDERS']['LLVMOpt'] = Builder(
        action='${LLVM_ROOT}/opt $LLVM_OPT_FLAGS $LLVM_OPT_PASSES -o=$TARGET $SOURCE')

    env['BUILDERS']['LLVMLink'] = Builder(
        action='${LLVM_ROOT}/llvm-link -o=$TARGET $SOURCES',
        emitter=add_libraries)
