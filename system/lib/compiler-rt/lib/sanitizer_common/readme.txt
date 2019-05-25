These files are from compiler-rt 8.0.0.

Some directories in compiler-rt are omitted here:

  * scripts
  * symbolizer
  * tests

===========================================================================

Changes were made to make this work with Emscripten:

* Define platform `SANITIZER_EMSCRIPTEN`.
* Add Emscripten-specific handling for `GetThreadStackTopAndBottom`,
  system calls, memory maps, colors.

For details, see <https://github.com/emscripten-core/emscripten/pull/8651>.
