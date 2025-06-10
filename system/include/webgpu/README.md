# WebGPU Bindings

**IMPORTANT NOTICE:**

Emscripten's WebGPU bindings `-sUSE_WEBGPU` are **deprecated and unmaintained**
and will be removed. Please migrate to Emdawnwebgpu via
[`--use-port=emdawnwebgpu`](../../../tools/ports/emdawnwebgpu.py):

> Emdawnwebgpu is a fork of Emscripten's original `USE_WEBGPU`, implementing a
> newer, more stable version of the standardized webgpu.h interface. If you
> find issues, verify in the latest nightly release
> (<https://github.com/google/dawn/releases>) and file feedback with Dawn.
> (Emdawnwebgpu is maintained as part of Dawn, the open-source WebGPU
> implementation used by Chromium, but it is still cross-browser.)

(Information about maintaining the `USE_WEBGPU` bindings has been removed,
since they won't be maintained anymore.)
