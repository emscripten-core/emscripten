#ifndef __em_asm_h__
#define __em_asm_h__


#define CONCAT(x, y) CONCAT_(x, y)
#define CONCAT_(x, y) x##y
#define CONCAT3(x, y, z) CONCAT(CONCAT(x, y), z)

#define NARG(...) NARG_(__VA_ARGS__, RSEQ_N())
#define NARG_(...) ARG_N(__VA_ARGS__)
#define ARG_N(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define RSEQ_N() 7, 6, 5, 4, 3, 2, 1, 0

#ifdef __cplusplus
template <typename... Args> void emscripten_asm_const(const char* code, Args...);
template <typename... Args> int emscripten_asm_const_int(const char* code, Args...);
template <typename... Args> double emscripten_asm_const_double(const char* code, Args...);
#else
extern void emscripten_asm_const(); \
extern int emscripten_asm_const_int(); \
extern double emscripten_asm_const_double();
#endif

extern void emscripten_asm_const(const char* code);

#define EM_ASM(code) emscripten_asm_const(#code)
#define EM_ASM_(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_ARGS(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_INT(code, ...) emscripten_asm_const_int(#code, __VA_ARGS__)
#define EM_ASM_DOUBLE(code, ...) emscripten_asm_const_double(#code, __VA_ARGS__)
#define EM_ASM_INT_V(code) emscripten_asm_const_int(#code)
#define EM_ASM_DOUBLE_V(code) emscripten_asm_const_double(#code)

#endif // __em_asm_h__
