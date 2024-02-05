.globaltype tlsVariable, i32
tlsVariable:

.globl get_tls_variable
get_tls_variable:
  .functype get_tls_variable () -> (i32)
  global.get tlsVariable
  end_function

.globl set_tls_variable
set_tls_variable:
  .functype set_tls_variable (i32) -> ()
  local.get 0
  global.set tlsVariable
  end_function
