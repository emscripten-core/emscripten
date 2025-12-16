#!/usr/bin/env python3
# Copyright 2023 The Emscripten Authors.  All rights reserved.
# Emscripten is available under two separate licenses, the MIT license and the
# University of Illinois/NCSA Open Source License.  Both these licenses can be
# found in the LICENSE file.

import os
import sys
import shutil
import subprocess


script_dir = os.path.abspath(os.path.dirname(__file__))
emscripten_root = os.path.dirname(os.path.dirname(script_dir))
default_llvm_dir = os.path.join(os.path.dirname(emscripten_root), "llvm-project")
local_root = os.path.join(script_dir, "libomp")
local_src = os.path.join(local_root, "src")
local_inc = os.path.join(local_root, "include")

excludes = ["doc", "build", "tests", "CMakeFiles"]


def clean_dir(dirname):
    for f in os.listdir(dirname):
        full = os.path.join(dirname, f)
        if os.path.isdir(full):
            shutil.rmtree(full)
        else:
            os.remove(full)


def copy_tree(upstream_dir, local_dir):
    for f in os.listdir(upstream_dir):
        full = os.path.join(upstream_dir, f)
        if f not in excludes:
            if os.path.isdir(full):
                if not os.path.exists(os.path.join(local_dir, f)):
                    os.makedirs(os.path.join(local_dir, f))
                copy_tree(full, os.path.join(local_dir, f))
            else:
                shutil.copy2(full, os.path.join(local_dir, f))


def main():
    if len(sys.argv) > 1:
        llvm_dir = os.path.join(os.path.abspath(sys.argv[1]))
    else:
        llvm_dir = default_llvm_dir
    upstream_root = os.path.join(llvm_dir, "openmp/")

    cwd = os.getcwd()
    os.chdir(upstream_root)
    if not os.path.exists("build"):
        os.mkdir("build")
    os.chdir("build")

    subprocess.call(
        [
            "emcmake",
            "cmake",
            "..",
            "-G",
            "Ninja",
            "-DOPENMP_STANDALONE_BUILD=ON",
            "-DOPENMP_ENABLE_LIBOMPTARGET=OFF",
            "-DLIBOMP_HAVE_OMPT_SUPPORT=OFF",
            "-DLIBOMP_OMPT_SUPPORT=OFF",
            "-DLIBOMP_OMPD_SUPPORT=OFF",
            "-DLIBOMP_USE_DEBUGGER=OFF",
            "-DLIBOMP_FORTRAN_MODULES=OFF",
            "-DLIBOMP_ENABLE_SHARED=OFF",
            "-DLIBOMP_ARCH=wasm32",
            "-DOPENMP_ENABLE_LIBOMPTARGET_PROFILING=OFF",
            "-DCMAKE_INSTALL_PREFIX=/",
        ]
    )
    subprocess.call("ninja")
    subprocess.call(["env", "DESTDIR=.", "ninja", "install"])
    os.chdir(cwd)

    upstream_runtime_root = os.path.join(upstream_root, "runtime/src")
    upstream_inc = os.path.join(upstream_root, "build/usr/include")
    upstream_build_src = os.path.join(upstream_root, "build/runtime/src")
    print(upstream_inc)
    assert os.path.exists(upstream_runtime_root)
    assert os.path.exists(upstream_inc)
    assert os.path.exists(upstream_build_src)

    # Remove old version
    # clean_dir(local_src)
    clean_dir(local_root)

    os.mkdir(local_src)
    os.mkdir(local_inc)

    copy_tree(upstream_runtime_root, local_src)
    copy_tree(upstream_inc, local_inc)

    shutil.copy2(os.path.join(upstream_root, "LICENSE.TXT"), local_root)
    shutil.copy2(
        os.path.join(upstream_build_src, "omp.h"),
        local_src,
    )
    shutil.copy2(
        os.path.join(upstream_build_src, "kmp_config.h"),
        local_src,
    )
    shutil.copy2(
        os.path.join(upstream_build_src, "kmp_i18n_id.inc"),
        local_src,
    )
    shutil.copy2(
        os.path.join(upstream_build_src, "kmp_i18n_default.inc"),
        local_src,
    )


if __name__ == "__main__":
    main()
