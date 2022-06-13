IMPORT_IMPL(void, Z_envZ_log_executionZ_vi, (u32 loc), {
  printf("log_execution %d\n", loc);
});
IMPORT_IMPL(u32, Z_envZ_get_i32Z_iiii, (u32 loc, u32 index, u32 value), {
  printf("get_i32 %d,%d,%d\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_get_i64Z_iiiii, (u32 loc, u32 index, u32 low, u32 high), {
  printf("get_i64 %d,%d,%d,%d\n", loc, index, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(f32, Z_envZ_get_f32Z_fiif, (u32 loc, u32 index, f32 value), {
  printf("get_f32 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_get_f64Z_diid, (u32 loc, u32 index, f64 value), {
  printf("get_f64 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_set_i32Z_iiii, (u32 loc, u32 index, u32 value), {
  printf("set_i32 %d,%d,%d\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_set_i64Z_iiiii, (u32 loc, u32 index, u32 low, u32 high), {
  printf("set_i64 %d,%d,%d,%d\n", loc, index, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(f32, Z_envZ_set_f32Z_fiif, (u32 loc, u32 index, f32 value), {
  printf("set_f32 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_set_f64Z_diid, (u32 loc, u32 index, f64 value), {
  printf("set_f64 %d,%d,%f\n", loc, index, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_load_ptrZ_iiiii, (u32 loc, u32 bytes, u32 offset, u32 ptr), {
  printf("load_ptr %d,%d,%d,%d\n", loc, bytes, offset, ptr);
  return ptr;
});
IMPORT_IMPL(u32, Z_envZ_load_val_i32Z_iii, (u32 loc, u32 value), {
  printf("load_val_i32 %d,%d\n", loc, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_load_val_i64Z_iiii, (u32 loc, u32 low, u32 high), {
  printf("load_val_i64 %d,%d,%d\n", loc, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(u64, Z_envZ_load_val_i64Z_jij, (u32 loc, u64 value), {
  printf("load_val_i64 %d,%d,%d\n", loc, (u32)value, (u32)(value >> 32));
  return value;
});
IMPORT_IMPL(f32, Z_envZ_load_val_f32Z_fif, (u32 loc, f32 value), {
  printf("load_val_f32 %d,%f\n", loc, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_load_val_f64Z_did, (u32 loc, f64 value), {
  printf("load_val_f64 %d,%f\n", loc, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_store_ptrZ_iiiii, (u32 loc, u32 bytes, u32 offset, u32 ptr), {
  printf("store_ptr %d,%d,%d,%d\n", loc, bytes, offset, ptr);
  return ptr;
});
IMPORT_IMPL(u32, Z_envZ_store_val_i32Z_iii, (u32 loc, u32 value), {
  printf("store_val_i32 %d,%d\n", loc, value);
  return value;
});
IMPORT_IMPL(u32, Z_envZ_store_val_i64Z_iiii, (u32 loc, u32 low, u32 high), {
  printf("store_val_i64 %d,%d,%d\n", loc, low, high);
  tempRet0 = high;
  return low;
});
IMPORT_IMPL(u64, Z_envZ_store_val_i64Z_jij, (u32 loc, u64 value), {
  printf("store_val_i64 %d,%d,%d\n", loc, (u32)value, (u32)(value >> 32));
  return value;
});
IMPORT_IMPL(f32, Z_envZ_store_val_f32Z_fif, (u32 loc, f32 value), {
  printf("store_val_f32 %d,%f\n", loc, value);
  return value;
});
IMPORT_IMPL(f64, Z_envZ_store_val_f64Z_did, (u32 loc, f64 value), {
  printf("store_val_f64 %d,%f\n", loc, value);
  return value;
});
