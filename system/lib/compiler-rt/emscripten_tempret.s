.section .globals,"",@

.globaltype tempRet0, i32
tempRet0:

.section .text,"",@

.globl setTempRet0
setTempRet0:
  .functype setTempRet0 (i32) -> ()
  local.get 0
  global.set tempRet0
  end_function

.globl getTempRet0
getTempRet0:
  .functype getTempRet0 () -> (i32)
  global.get tempRet0
  end_function

# These aliases exist solely for LegalizeJSInterface pass in binaryen
# They get exported by emcc and the exports are then removed by the
# binaryen pass
.globl __get_temp_ret
.type __get_temp_ret, @function
__get_temp_ret = getTempRet0

.globl __set_temp_ret
.type __set_temp_ret, @function
__set_temp_ret = setTempRet0
