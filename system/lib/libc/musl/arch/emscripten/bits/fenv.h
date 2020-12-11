#define FE_ALL_EXCEPT 0

#define FE_TONEAREST  0
#define FE_DOWNWARD   0x400
#define FE_UPWARD     0x800
#define FE_TOWARDZERO 0xc00

typedef unsigned short fexcept_t;
typedef unsigned short fenv_t;

#define FE_DFL_ENV      ((const fenv_t *) -1)
