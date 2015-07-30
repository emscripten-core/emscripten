function a(x, y) {
 x = SIMD_int32x4_check(x);
 y = SIMD_float32x4_check(y);
 var z = SIMD_float32x4(0, 0, 0, 0);
 work(z);
}

