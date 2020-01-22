These files are from compiler-rt 8.0.0.

===========================================================================

Changes were made to make this work with Emscripten:

* Recognize __EMSCRIPTEN__ as a platform that UBSan can be used on
* Let Emscripten run without signal interception (browsers don't have them)

For details, see <https://github.com/emscripten-core/emscripten/pull/8651>.
