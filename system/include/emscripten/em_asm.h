#ifndef __em_asm_h__
#define __em_asm_h__


#define CONCAT(x, y) CONCAT_(x, y)
#define CONCAT_(x, y) x##y
#define CONCAT3(x, y, z) CONCAT(CONCAT(x, y), z)

#define NARG(...) NARG_(__VA_ARGS__, RSEQ_N())
#define NARG_(...) ARG_N(__VA_ARGS__)
#define ARG_N(_1, _2, _3, _4, _5, _6, _7, N, ...) N
#define RSEQ_N() 7, 6, 5, 4, 3, 2, 1, 0

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)
#define PROBE(x) x, 1,

#define NOT(x) CHECK(CONCAT(NOT_, x))
#define NOT_0 PROBE(~)

#define COMPL(b) CONCAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define IIF(b) CONCAT(IIF_, b)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t
#define IF(cond) IIF(COMPL(NOT(cond)))

#ifdef __cplusplus
#define EM_DECL_REC_0(base, ...) \
    base(const char* code);
#define EM_DECL_REC_1(base, ...) \
    base(const char* code, __VA_ARGS__);
#define EM_DECL_REC_2(base, ...) \
    EM_DECL_REC_1(base, int _1, __VA_ARGS__) \
    EM_DECL_REC_1(base, double _1, __VA_ARGS__)
#define EM_DECL_REC_3(base, ...) \
    EM_DECL_REC_2(base, int _2, __VA_ARGS__) \
    EM_DECL_REC_2(base, double _2, __VA_ARGS__)
#define EM_DECL_REC_4(base, ...) \
    EM_DECL_REC_3(base, int _3, __VA_ARGS__) \
    EM_DECL_REC_3(base, double _3, __VA_ARGS__)
#define EM_DECL_REC_5(base, ...) \
    EM_DECL_REC_4(base, int _4, __VA_ARGS__) \
    EM_DECL_REC_4(base, double _4, __VA_ARGS__)
#define EM_DECL_REC_6(base, ...) \
    EM_DECL_REC_5(base, int _5, __VA_ARGS__) \
    EM_DECL_REC_5(base, double _5, __VA_ARGS__)
#define EM_DECL_REC_7(base, ...) \
    EM_DECL_REC_6(base, int _6, __VA_ARGS__) \
    EM_DECL_REC_6(base, double _6, __VA_ARGS__)
#define EM_DECL_REC_N(N, base) \
    CONCAT(EM_DECL_REC_, N)(base, int _N) \
    CONCAT(EM_DECL_REC_, N)(base, double _N)
#define EM_DECL(N) \
    EM_DECL_REC_N(N, extern void emscripten_asm_const_##N) \
    EM_DECL_REC_N(N, extern int emscripten_asm_const_int_##N) \
    EM_DECL_REC_N(N, extern double emscripten_asm_const_double_##N)
#else
#define EM_DECL(N) \
    extern void emscripten_asm_const_##N(); \
    extern int emscripten_asm_const_int_##N(); \
    extern double emscripten_asm_const_double_##N();
#endif
#define EM_ASM_IMPL_1(base, code, ...) CONCAT3(base, _, NARG(__VA_ARGS__))(#code, __VA_ARGS__)
#define EM_ASM_IMPL_0(base, code) CONCAT(base, _0)(#code)
#define EM_ASM_IMPL(base, code, ...) IF(CHECK(__VA_ARGS__))(\
    EM_ASM_IMPL_1(base, code, __VA_ARGS__),\
    EM_ASM_IMPL_0(base, code))
extern void emscripten_asm_const(const char* code);

#define EM_ASM(...) emscripten_asm_const(#__VA_ARGS__)
#define EM_ASM_(...) EM_ASM_IMPL(emscripten_asm_const_int, __VA_ARGS__)
#define EM_ASM_ARGS(...) EM_ASM_IMPL(emscripten_asm_const_int, __VA_ARGS__)
#define EM_ASM_INT(...) EM_ASM_IMPL(emscripten_asm_const_int, __VA_ARGS__)
#define EM_ASM_DOUBLE(...) EM_ASM_IMPL(emscripten_asm_const_double, __VA_ARGS__)
#define EM_ASM_INT_V(code) EM_ASM_IMPL(emscripten_asm_const_int, code)
#define EM_ASM_DOUBLE_V(code) EM_ASM_IMPL(emscripten_asm_const_double, code)

EM_DECL(0); EM_DECL(1); EM_DECL(2); EM_DECL(3);
EM_DECL(4); EM_DECL(5); EM_DECL(6); EM_DECL(7);

#endif // __em_asm_h__
