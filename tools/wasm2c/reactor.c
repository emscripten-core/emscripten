// TODO: optional prefixing
void WASM_RT_ADD_PREFIX(wasmbox_init)(void) {
  // Initialize wasm2c runtime.
  WASM_RT_ADD_PREFIX(init)();

  // Set up handling for a trap
  int trap_code;
  if ((trap_code = setjmp(g_jmp_buf))) {
    printf("[wasm trap %d, halting]\n", trap_code);
    abort();
  } else {
    WASM_RT_ADD_PREFIX(Z__initializeZ_vv)();
  }
}
