// XXX Emscripten in sync with musl/arch/x86_64/bits/fenv.h expect for these defines:
// #define FE_INVALID    1
// #define __FE_DENORM   2
// #define FE_DIVBYZERO  4
// #define FE_OVERFLOW   8
// #define FE_UNDERFLOW  16
// #define FE_INEXACT    32
//
// #define FE_ALL_EXCEPT 63
#define FE_ALL_EXCEPT 0 // XXX Emscripten in sync with musl/arch/generic/bits/fenv.h
// TODO(kleisauke): Perhaps we should sync this with musl/arch/i386/bits/fenv.h instead?

#define FE_TONEAREST  0
#define FE_DOWNWARD   0x400
#define FE_UPWARD     0x800
#define FE_TOWARDZERO 0xc00

typedef unsigned short fexcept_t;

typedef struct {
	unsigned short __control_word;
	unsigned short __unused1;
	unsigned short __status_word;
	unsigned short __unused2;
	unsigned short __tags;
	unsigned short __unused3;
	unsigned int __eip;
	unsigned short __cs_selector;
	unsigned int __opcode:11;
	unsigned int __unused4:5;
	unsigned int __data_offset;
	unsigned short __data_selector;
	unsigned short __unused5;
	unsigned int __mxcsr;
} fenv_t;

#define FE_DFL_ENV      ((const fenv_t *) -1)
