/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <emscripten.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

int main(int argc, char *argv[])
{
  EM_ASM({
    var counter = FS.makedev(64, 0);

    FS.registerDevice(counter, {
      open: function(stream) {},
      close: function(stream) {},
#if WASMFS
      // WasmFS has different requirements for a device's functions compared
      // to the legacy API.
      read: function(file, buffer, length, offset) {
        // WasmFS does not provide the file's current seek position to 
        // custom devices, unlike the legacy FS.
        var position = 0x10000005A; // We hardcode this value for testing purposes.
        var tempBuffer = [];
        for (var i = 0; i < length; i++) {
          tempBuffer.push(i + position);
        }
        Module.HEAP8.set(tempBuffer, buffer);
        return length;
      }
#else
      read: function(stream, buffer, offset, length, position) {
        for (var i = 0; i < length; ++i) {
          buffer[offset + i] = position + i;
        }
        return length;
      },
      llseek: function(stream, offset, whence) {
        var position = offset;
        if (whence === 1) {
          position += stream.position;
        }
        return position;
      }
#endif
    });

    FS.mkdev('/counter', counter);
  });

  FILE* file = fopen("/counter", "rb");
  assert(file != NULL);
  fseeko(file, 0x10000005A, SEEK_SET);
  assert(fgetc(file) == 0x5A);
  fclose(file);

  puts("success");
  return 0;
}
