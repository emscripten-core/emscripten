function a(x, y) {
 x = SIMD_Int32x4_check(x);
 y = SIMD_Float32x4_check(y);
 var z = SIMD_Float32x4(0, 0, 0, 0);
 work(z);
}

