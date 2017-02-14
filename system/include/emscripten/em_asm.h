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
#define EM_DECL_REC_0(base, ...) \
    base(const char* code);
#define EM_DECL_REC_1(base, ...) \
    base(const char* code, __VA_ARGS__);
#define EM_DECL_REC_2(base, ...) \
    EM_DECL_REC_1(base, int _1, __VA_ARGS__) \
    EM_DECL_REC_1(base, double _1, __VA_ARGS__) \
    EM_DECL_REC_1(base, const char* _1, __VA_ARGS__)
#define EM_DECL_REC_3(base, ...) \
    EM_DECL_REC_2(base, int _2, __VA_ARGS__) \
    EM_DECL_REC_2(base, double _2, __VA_ARGS__) \
    EM_DECL_REC_2(base, const char* _2, __VA_ARGS__)
#define EM_DECL_REC_4(base, ...) \
    EM_DECL_REC_3(base, int _3, __VA_ARGS__) \
    EM_DECL_REC_3(base, double _3, __VA_ARGS__) \
    EM_DECL_REC_3(base, const char* _3, __VA_ARGS__)
#define EM_DECL_REC_5(base, ...) \
    EM_DECL_REC_4(base, int _4, __VA_ARGS__) \
    EM_DECL_REC_4(base, double _4, __VA_ARGS__) \
    EM_DECL_REC_4(base, const char* _4, __VA_ARGS__)
#define EM_DECL_REC_6(base, ...) \
    EM_DECL_REC_5(base, int _5, __VA_ARGS__) \
    EM_DECL_REC_5(base, double _5, __VA_ARGS__) \
    EM_DECL_REC_5(base, const char* _5, __VA_ARGS__)
#define EM_DECL_REC_7(base, ...) \
    EM_DECL_REC_6(base, int _6, __VA_ARGS__) \
    EM_DECL_REC_6(base, double _6, __VA_ARGS__) \
    EM_DECL_REC_6(base, const char* _6, __VA_ARGS__)
#define EM_DECL_REC_N(N, base) \
    CONCAT(EM_DECL_REC_, N)(base, int _N) \
    CONCAT(EM_DECL_REC_, N)(base, double _N) \
    CONCAT(EM_DECL_REC_, N)(base, const char* _N)
#define EM_DECL(N) \
    EM_DECL_REC_N(N, void emscripten_asm_const) \
    EM_DECL_REC_N(N, int emscripten_asm_const_int) \
    EM_DECL_REC_N(N, double emscripten_asm_const_double)

EM_DECL(0); EM_DECL(1); EM_DECL(2); EM_DECL(3);
// EM_DECL(4); EM_DECL(5); EM_DECL(6); EM_DECL(7);
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
