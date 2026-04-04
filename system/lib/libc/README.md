This folder contains the musl version of libc at `/musl`. The upstream version
can be found at http://www.musl-libc.org/.

Most of the source comes from musl v1.2.5, with some exceptions listed below.
We track these changes from upstream in https://github.com/emscripten-core/musl
and use a script (`system/lib/update_musl.py`) to pull in updates.

Some changes have been made to the version that was taken from upstream, including:

 * Emscripten-specific changes (from before this readme existed). These should be marked with `XXX EMSCRIPTEN` in the source, or ifdefed with `#if __EMSCRIPTEN__`. They are mostly in pthreads code and hopefully temporary.
 * Backporting an operator-precedence warning fix from 6e76e1540fc58a418494bf5eb832b556f9c5763e in the upstream version.
 * Switch to using the wasi `fd_write` syscall instead of `writev`.
 * Simplify stdout stream handling: do not support seeking, terminal handling, etc., as it just increases code size and Emscripten doesn't have those features anyhow.
 * Setting `_POSIX_REALTIME_SIGNALS` and `_POSIX_SPAWN` macros to -1, to exclude unsupported functions.
 * Handling trailing % in `strftime` and `wcsftime` format strings.

Copy log.c and log2.c from earlier version of musl which result in smaller
binary size since they do not rely on data tables in log_data.c and log2_data.c.
See https://github.com/emscripten-core/emscripten/issues/15483.

Verifying upstream musl behaviour
=================================

Occasionally when working on libc/musl it can be useful to verify to behavior
of upstream musl.  For example, when trying to determine if a certain behavior
is a bug in emscripten, or an upstream bug, or just expected musl behavior.

When I need to do this I use the Alpine linux docker image.  Alpine linux is a
distro where the sysmtem libc is musl so by default any program you build within
the contains will be using musl libc.

    $ docker run --rm -it -v "$(pwd):/data" alpine /bin/sh

Then from inside the new container you can run tests against Alpine's musl libc.
For example:

    $ apk add build-base
    $ cd /data
    $ gcc -pthread test/pthread/test_pthread_cancel_async.c
    $ ./a.out
