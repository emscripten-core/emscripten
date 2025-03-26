#ifndef _INTERNAL_RELOC_H
#define _INTERNAL_RELOC_H

#include <features.h>
#include <elf.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifdef __EMSCRIPTEN__
// Declare `struct dso` in this header so that it is visible to gen_struct_info.

#include <emscripten/emscripten.h>

struct dso {
  // Pointer back to the dlevent in the event sequence which loaded this DSO.
  struct dlevent* event;

  // Flags used to open the library.  We need to cache these so that other
  // threads can mirror the open library state.
  int flags;

  // Location in memory/table of static data/static function addresses
  // The first thread to load a given module alloces the memory and table
  // address space and then sets this field to non-zero.
  uint8_t mem_allocated;
  void* mem_addr;
  size_t mem_size;
  void* table_addr;
  size_t table_size;

  // For DSO load events, where the DSO comes from a file on disc, this
  // is a pointer the file data read in by the laoding thread and shared with
  // others.
  uint8_t* file_data;
  size_t file_data_size;

  // Flexible array; must be final element of struct
  char name[];
};

#else

#if UINTPTR_MAX == 0xffffffff
typedef Elf32_Ehdr Ehdr;
typedef Elf32_Phdr Phdr;
typedef Elf32_Sym Sym;
#define R_TYPE(x) ((x)&255)
#define R_SYM(x) ((x)>>8)
#define R_INFO ELF32_R_INFO
#else
typedef Elf64_Ehdr Ehdr;
typedef Elf64_Phdr Phdr;
typedef Elf64_Sym Sym;
#define R_TYPE(x) ((x)&0x7fffffff)
#define R_SYM(x) ((x)>>32)
#define R_INFO ELF64_R_INFO
#endif

/* These enum constants provide unmatchable default values for
 * any relocation type the arch does not use. */
enum {
	REL_NONE = 0,
	REL_SYMBOLIC = -100,
	REL_USYMBOLIC,
	REL_GOT,
	REL_PLT,
	REL_RELATIVE,
	REL_OFFSET,
	REL_OFFSET32,
	REL_COPY,
	REL_SYM_OR_REL,
	REL_DTPMOD,
	REL_DTPOFF,
	REL_TPOFF,
	REL_TPOFF_NEG,
	REL_TLSDESC,
	REL_FUNCDESC,
	REL_FUNCDESC_VAL,
};

struct fdpic_loadseg {
	uintptr_t addr, p_vaddr, p_memsz;
};

struct fdpic_loadmap {
	unsigned short version, nsegs;
	struct fdpic_loadseg segs[];
};

struct fdpic_dummy_loadmap {
	unsigned short version, nsegs;
	struct fdpic_loadseg segs[1];
};

#include "reloc.h"

#ifndef FDPIC_CONSTDISP_FLAG
#define FDPIC_CONSTDISP_FLAG 0
#endif

#ifndef DL_FDPIC
#define DL_FDPIC 0
#endif

#ifndef DL_NOMMU_SUPPORT
#define DL_NOMMU_SUPPORT 0
#endif

#ifndef TLSDESC_BACKWARDS
#define TLSDESC_BACKWARDS 0
#endif

#if !DL_FDPIC
#define IS_RELATIVE(x,s) ( \
	(R_TYPE(x) == REL_RELATIVE) || \
	(R_TYPE(x) == REL_SYM_OR_REL && !R_SYM(x)) )
#else
#define IS_RELATIVE(x,s) ( ( \
	(R_TYPE(x) == REL_FUNCDESC_VAL) || \
	(R_TYPE(x) == REL_SYMBOLIC) ) \
	&& (((s)[R_SYM(x)].st_info & 0xf) == STT_SECTION) )
#endif

#ifndef NEED_MIPS_GOT_RELOCS
#define NEED_MIPS_GOT_RELOCS 0
#endif

#ifndef DT_DEBUG_INDIRECT
#define DT_DEBUG_INDIRECT 0
#endif

#ifndef DT_DEBUG_INDIRECT_REL
#define DT_DEBUG_INDIRECT_REL 0
#endif

#define AUX_CNT 32
#define DYN_CNT 37

typedef void (*stage2_func)(unsigned char *, size_t *);

#endif // __EMSCRIPTEN__

hidden void *__dlsym(void *restrict, const char *restrict, void *restrict);

hidden void __dl_seterr(const char *, ...);
hidden int __dl_invalid_handle(void *);
hidden void __dl_vseterr(const char *, va_list);

hidden ptrdiff_t __tlsdesc_static(), __tlsdesc_dynamic();

hidden extern int __malloc_replaced;
hidden extern int __aligned_alloc_replaced;
hidden void __malloc_donate(char *, char *);
hidden int __malloc_allzerop(void *);

#endif
