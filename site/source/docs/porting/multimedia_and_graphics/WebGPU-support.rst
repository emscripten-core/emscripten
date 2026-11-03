.. _WebGPU-support:

==========================
Using WebGPU in Emscripten
==========================

Emscripten supports using WebGPU via `Emdawnwebgpu <https://dawn.googlesource.com/dawn/+/refs/heads/main/src/emdawnwebgpu/pkg/>`_,
an external Emscripten port that implements a `Dawn <https://dawn.googlesource.com/dawn>`_-like version of the
`webgpu.h <https://github.com/webgpu-native/webgpu-headers/>`_ C API, on top of the browser's WebGPU.
See the Emdawnwebgpu link for documentation, and
`this article <https://developer.chrome.com/docs/web-platform/webgpu/build-app>`_ for an example of how to use it.
