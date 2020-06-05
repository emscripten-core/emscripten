// TODO: optional prefixing
void wasmbox_init(void) {
  // Initialize wasm2c runtime.
  init();

  // Set up handling for a trap
  int trap_code;
  if ((trap_code = setjmp(g_jmp_buf))) {
    printf("[wasm trap %d, halting]\n", trap_code);
    abort();
  } else {
    Z__initializeZ_vv();
  }
}
