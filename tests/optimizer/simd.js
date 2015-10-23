function a(x, y) {
 x = SIMD_Int32x4_check(x);
 y = SIMD_Float32x4_check(y);
 var z = SIMD_Float32x4(0, 0, 0, 0);
 work(z);
}
function b(i, j) {
 i = i | 0;
 j = j | 0;
 var I = SIMD_Int32x4(0, 0, 0, 0), k = 0;
 I = SIMD_Int32x4_add(SIMD_Int32x4_splat(Math_fround(0)), SIMD_Int32x4(0, 1, 2, 3));
 k = i + (j << 2) | 0;
 SIMD_Int32x4_store(HEAPU8, k, I);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a", "b"]

