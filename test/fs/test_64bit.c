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
