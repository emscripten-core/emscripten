import os

deps = ['libsquashfs']

port_name = 'emscripten-wasmfs-squashfs'

root_path = os.path.dirname(os.path.realpath(__file__))

def needed(settings):
  return settings.USE_EMSCRIPTEN_WASMFS_SQUASHFS

def get_lib_name(settings):
    return 'emscripten_wasmfs_squashfs.a'

def get(ports, settings, shared):
    def create(final):
        emscripten_root = shared.path_from_root()
        source_path = os.path.join(root_path, 'emscripten_wasmfs_squashfs', 'src')
        source_include_paths = [os.path.join(root_path, 'emscripten_wasmfs_squashfs', 'include'), os.path.join(emscripten_root, 'system', 'lib', 'wasmfs')]
        srcs = [os.path.join(source_path, "backend.cpp")]
        flags = ['-sUSE_LIBSQUASHFS']
        ports.build_port(source_path, final, port_name, includes=source_include_paths, srcs=srcs, flags=flags)
    return [shared.cache.get_lib(get_lib_name(settings), create, what='port')]

def clear(ports, settings, shared):
    shared.cache.erase_lib(get_lib_name(settings))




