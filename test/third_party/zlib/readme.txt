This is zlib 1.2.5. See README for licensing info.

Changes for emscripten:

  deflate.c: Initialize match_start to 0, to prevent a SAFE_HEAP notification
             Initialize s->prev's buffer (in 2 places) to 0, same reasons

  example.c: Use %d instead of %x in version number printout
             Comment out gzio test

