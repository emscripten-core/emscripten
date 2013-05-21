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
    env.SetDefault(
        CLANG='clang',
        CLANGXX='clang++',
        LLVM_DIS='llvm-dis',
        LLVM_OPT='opt',
        LLVM_LINK='llvm-link')
    
    env['BUILDERS']['LLVMDis'] = Builder(
        action='${LLVM_ROOT}/$LLVM_DIS -o=$TARGET $SOURCE')

    env['BUILDERS']['LLVMOpt'] = Builder(
        action='${LLVM_ROOT}/$LLVM_OPT $LLVM_OPT_FLAGS $LLVM_OPT_PASSES -o=$TARGET $SOURCE')

    env['BUILDERS']['LLVMLink'] = Builder(
        action='${LLVM_ROOT}/$LLVM_LINK -o=$TARGET $SOURCES',
        emitter=add_libraries)
