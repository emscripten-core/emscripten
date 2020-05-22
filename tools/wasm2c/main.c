static int main_argc;
static char** main_argv;

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_args_sizes_getZ_iii, (u32 pargc, u32 pargv_buf_size), {
  wasm_i32_store(pargc, main_argc);
  u32 buf_size = 0;
  for (u32 i = 0; i < main_argc; i++) {
    buf_size += strlen(main_argv[i]) + 1;
  }
  wasm_i32_store(pargv_buf_size, buf_size);
  return 0;
});

IMPORT_IMPL(u32, Z_wasi_snapshot_preview1Z_args_getZ_iii, (u32 argv, u32 argv_buf), {
  u32 buf_size = 0;
  for (u32 i = 0; i < main_argc; i++) {
    u32 ptr = argv_buf + buf_size;
    wasm_i32_store(argv + i * 4, ptr);
    char* arg = main_argv[i];
    strcpy(MEMACCESS(ptr), arg);
    buf_size += strlen(arg) + 1;
  }
  return 0;
});

int main(int argc, char** argv) {
  main_argc = argc;
  main_argv = argv;

  init_fds();

  init();

  int trap_code;
  if ((trap_code = setjmp(g_jmp_buf))) {
    printf("[wasm trap %d, halting]\n", trap_code);
    return 1;
  } else {
    Z__startZ_vv();
  }
  return 0;
}
