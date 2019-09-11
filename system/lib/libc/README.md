This folder contains the musl version of libc at `/musl`.  The upstream version can be found at http://www.musl-libc.org/

Some changes have been made to the version that was taken from upstream, including:

 * Emscripten-specific changes (from before this readme existed). These should be marked with `XXX EMSCRIPTEN` in the source, or ifdefed with `#if __EMSCRIPTEN__`. They are mostly in pthreads code and hopefully temporary.
 * Backporting an operator-precedence warning fix from 6e76e1540fc58a418494bf5eb832b556f9c5763e in the upstream version.
 * Switch to using the wasi `fd_write` syscall instead of `writev`.
 * Simplify stdout stream handling: do not support seeking, terminal handling, etc., as it just increases code size and Emscripten doesn't have those features anyhow.

