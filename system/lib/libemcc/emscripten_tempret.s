.section .globals,"",@

.globaltype tempRet0, i32
tempRet0:

.section .text,"",@

.globl _emscripten_tempret_set
_emscripten_tempret_set:
  .functype _emscripten_tempret_set (i32) -> ()
  local.get 0
  global.set tempRet0
  end_function

.globl _emscripten_tempret_get
_emscripten_tempret_get:
  .functype _emscripten_tempret_get () -> (i32)
  global.get tempRet0
  end_function

# These aliases exist for LegalizeJSInterface pass in binaryen
# They get exported by emcc and the exports are then removed by the
# binaryen pass
.globl __get_temp_ret
.type __get_temp_ret, @function
__get_temp_ret = _emscripten_tempret_get

.globl __set_temp_ret
.type __set_temp_ret, @function
__set_temp_ret = _emscripten_tempret_set

# These aliases exist for the llvm passes that generate calls to these
# functions.  These aliases should no longer be exported by emscripten
# and hopefully can be removed if/when we transition llvm to the new names
# above.
.globl getTempRet0
.type getTempRet0, @function
getTempRet0 = _emscripten_tempret_get

.globl setTempRet0
.type setTempRet0, @function
setTempRet0 = _emscripten_tempret_set
