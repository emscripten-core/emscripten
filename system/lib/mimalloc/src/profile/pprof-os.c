/* ----------------------------------------------------------------------------
Copyright (c) 2018-2026, Microsoft Research, Daan Leijen
This is free software; you can redistribute it and/or modify it under the
terms of the MIT license. A copy of the license can be found in the file
"LICENSE" at the root of this distribution.
-----------------------------------------------------------------------------*/

/* ----------------------------------------------------------------------------
Disclaimer: Unlike the rest of the considerate hand-crafted mimalloc sources,
this file was written with AI assistance (for the process image).
-----------------------------------------------------------------------------*/

// OS specific primitives of the pprof profiler: wall-clock time, opening snapshot files,
// backtraces, and the address ranges of the loaded modules.

#include "mimalloc.h"
#include "mimalloc/util/libc.h"
#include "mimalloc/util/options.h"
#include "pprof.h"

#if !MI_PROFILE_USE_BUILTIN

size_t _mi_pprof_backtrace(void** buffer, size_t max_depth, size_t* hash) {
  MI_UNUSED(buffer);
  MI_UNUSED(max_depth);
  *hash = 0;
  return 0;
}

int64_t _mi_pprof_time_secs(void) {
  return 0;
}

#else

#include <time.h>       // time
#if defined(_WIN32)
#include <fcntl.h>      // _O_CREAT, _O_TRUNC, _O_WRONLY, _O_BINARY, _O_TEXT
#include <io.h>         // _sopen_s, _write, _close
#include <share.h>      // _SH_DENYNO
#include <sys/stat.h>   // _S_IREAD, _S_IWRITE
#elif MI_HAS_UNISTDH
#include <errno.h>      // EINTR
#include <fcntl.h>      // open, O_CREAT, O_NOFOLLOW, O_CLOEXEC
#include <unistd.h>     // write, read, close
#include <sys/stat.h>   // S_IRUSR, S_IWUSR
#else
#include <stdio.h>      // FILE, fopen, fwrite, fclose
#endif


int64_t _mi_pprof_time_secs(void) {
  const time_t t = time(NULL);
  return (t == (time_t)-1 ? 0 : (int64_t)t);
}


// ---------------------------------------------------------------------------
// Snapshot files
//
// A snapshot can be taken from within an allocation (for interval snapshots in `on_alloc`),
// and that allocation may be called from within the C library's stdio (e.g. when `puts`
// allocates its buffer). As stdio is not reentrant, we never use it here but write
// directly to a file descriptor through our own buffer (allocated in the profiler's heap).
// ---------------------------------------------------------------------------

#define MI_PPROF_FILE_BUF_SIZE  (16*MI_KiB)

struct mi_pprof_file_s {
  mi_heap_t* heap;
  #if defined(_WIN32) || MI_HAS_UNISTDH
  int        fd;
  #else
  FILE*      fp;
  #endif
  bool       failed;  // set if any write failed
  size_t     len;     // bytes used in `buf`
  uint8_t    buf[MI_PPROF_FILE_BUF_SIZE];
};

// Open a snapshot file for writing, refusing to follow a symlink at `fname` (to avoid an
// attacker redirecting the write to an arbitrary target) and restricting the file's
// permissions to the owner only (snapshots can contain sensitive address/callstack data).
// On platforms without these POSIX/Win32 facilities, we fall back to a plain `fopen`
// (without the symlink/permission hardening and without protection against stdio reentrancy).
mi_pprof_file_t* _mi_pprof_fopen(mi_heap_t* heap, const char* fname, bool binary) {
  mi_pprof_file_t* const f = (mi_pprof_file_t*)mi_heap_malloc(heap, sizeof(mi_pprof_file_t));
  if (f == NULL) return NULL;
  f->heap = heap;
  f->failed = false;
  f->len = 0;
  #if defined(_WIN32)
    int oflag = _O_CREAT | _O_TRUNC | _O_WRONLY | (binary ? _O_BINARY : _O_TEXT);
    int fd = -1;
    if (_sopen_s(&fd, fname, oflag, _SH_DENYNO, _S_IREAD | _S_IWRITE) != 0 || fd < 0) { mi_free(f); return NULL; }
    f->fd = fd;
  #elif MI_HAS_UNISTDH
    MI_UNUSED(binary);
    int oflag = O_CREAT | O_TRUNC | O_WRONLY;
    #if defined(O_NOFOLLOW)
    oflag |= O_NOFOLLOW;   // fail (ELOOP) if `fname` is a symlink
    #endif
    #if defined(O_CLOEXEC)
    oflag |= O_CLOEXEC;
    #endif
    const int fd = open(fname, oflag, S_IRUSR | S_IWUSR);  // restrict to owner read/write only
    if (fd < 0) { mi_free(f); return NULL; }
    f->fd = fd;
  #else
    f->fp = fopen(fname, binary ? "wb" : "w");
    if (f->fp == NULL) { mi_free(f); return NULL; }
  #endif
  return f;
}

static void mi_pprof_file_flush(mi_pprof_file_t* f) {
  const uint8_t* p = f->buf;
  size_t n = f->len;
  f->len = 0;
  while (n > 0 && !f->failed) {
    #if defined(_WIN32)
    const int w = _write(f->fd, p, (unsigned int)(n > MI_PPROF_FILE_BUF_SIZE ? MI_PPROF_FILE_BUF_SIZE : n));
    #elif MI_HAS_UNISTDH
    const ssize_t w = write(f->fd, p, n);
    if (w < 0 && errno == EINTR) continue;
    #else
    const size_t w = fwrite(p, 1, n, f->fp);
    #endif
    if (w <= 0) { f->failed = true; break; }
    p += (size_t)w;
    n -= (size_t)w;
  }
}

void _mi_pprof_fwrite(mi_pprof_file_t* f, const void* data, size_t size) {
  const uint8_t* p = (const uint8_t*)data;
  while (size > 0 && !f->failed) {
    if (f->len == MI_PPROF_FILE_BUF_SIZE) { mi_pprof_file_flush(f); }
    const size_t avail = MI_PPROF_FILE_BUF_SIZE - f->len;
    const size_t n = (size < avail ? size : avail);
    mi_memcpy(f->buf + f->len, p, n);
    f->len += n;
    p += n;
    size -= n;
  }
}

void _mi_pprof_fputs(mi_pprof_file_t* f, const char* s) {
  _mi_pprof_fwrite(f, s, _mi_strlen(s));
}

// Flush and close the file; returns `false` if any write failed.
bool _mi_pprof_fclose(mi_pprof_file_t* f) {
  mi_pprof_file_flush(f);
  bool ok = !f->failed;
  #if defined(_WIN32)
  if (_close(f->fd) != 0) { ok = false; }
  #elif MI_HAS_UNISTDH
  if (close(f->fd) != 0) { ok = false; }
  #else
  if (fclose(f->fp) != 0) { ok = false; }
  #endif
  mi_free(f);
  return ok;
}

// ---------------------------------------------------------------------------
// Basic backtraces
// ---------------------------------------------------------------------------

#if MI_HAS_EXECINFOH || MI_HAS_UNWINDH
static size_t mi_backtrace_hash(void** frames, size_t depth) {
  size_t hash = MI_FNV_OFFSET_BASIS;
  for (size_t i = 0; i < depth; i++) {
    hash = (hash ^ (size_t)((uintptr_t)frames[i])) * MI_FNV_PRIME;
  }
  return hash;
}
#endif

#if _WIN32
#include <windows.h>    // CaptureStackBackTrace
mi_decl_noinline size_t _mi_pprof_backtrace(void** buffer, size_t max_depth, size_t* hash) {
  if (max_depth > ULONG_MAX) { max_depth = ULONG_MAX; }
  // On Windows XP/Server 2003, `FramesToSkip + FramesToCapture` must be less than 63;
  // clamp defensively so we never exceed this, regardless of the actual OS version.
  const size_t skip = 1 + MI_BACKTRACE_SKIP_FRAMES;  // +1 for `_mi_pprof_backtrace`'s own frame
  const size_t win_limit = 62;                       // strictly less than 63
  if (skip + max_depth >= win_limit) { max_depth = (skip <= win_limit ? win_limit - skip : 0); }
  if (max_depth == 0) { *hash = 0; return 0; }
  DWORD win_hash = 0;
  const size_t depth = (size_t)CaptureStackBackTrace((ULONG)skip, (ULONG)max_depth, buffer, &win_hash);
  *hash = (size_t)win_hash;  // use the hash computed by `CaptureStackBackTrace` itself
  return depth;
}
#elif MI_HAS_EXECINFOH
#include <execinfo.h>   // backtrace
mi_decl_noinline size_t _mi_pprof_backtrace(void** buffer, size_t max_depth, size_t* hash) {
  const size_t skip = 1 + MI_BACKTRACE_SKIP_FRAMES;  // +1 for `_mi_pprof_backtrace`'s own frame
  if (max_depth <= skip) { *hash = 0; return 0; }
  if (max_depth > INT_MAX) { max_depth = INT_MAX; }
  const size_t raw_depth = (size_t)backtrace(buffer, (int)(max_depth - skip));
  if (raw_depth <= skip) { *hash = 0; return 0; }
  const size_t depth = raw_depth - skip;
  for (size_t i = 0; i < depth; i++) { buffer[i] = buffer[i + skip]; }  // overlapping move down
  *hash = mi_backtrace_hash(buffer, depth);  // no platform-provided hash, compute our own
  return depth;
}
#elif MI_HAS_UNWINDH
#include <unwind.h>     // _Unwind_Backtrace (from the compiler's unwinder: libgcc_s or LLVM libunwind)
typedef struct mi_unwind_state_s {
  void** buffer;
  size_t max_depth;
  size_t skip;
  size_t depth;
} mi_unwind_state_t;

static _Unwind_Reason_Code mi_unwind_frame(struct _Unwind_Context* ctx, void* arg) {
  mi_unwind_state_t* const st = (mi_unwind_state_t*)arg;
  const uintptr_t ip = (uintptr_t)_Unwind_GetIP(ctx);
  if (ip == 0) return _URC_END_OF_STACK;
  if (st->skip > 0) { st->skip--; return _URC_NO_REASON; }
  st->buffer[st->depth++] = (void*)ip;
  return (st->depth >= st->max_depth ? _URC_END_OF_STACK : _URC_NO_REASON);
}

mi_decl_noinline size_t _mi_pprof_backtrace(void** buffer, size_t max_depth, size_t* hash) {
  // used as a fallback on platforms without `execinfo.h` (e.g. musl-based Linux like Alpine)
  mi_unwind_state_t st = { buffer, max_depth, 1 + MI_BACKTRACE_SKIP_FRAMES /* +1 for `_mi_pprof_backtrace`'s own frame */, 0 };
  if (max_depth > 0) { _Unwind_Backtrace(&mi_unwind_frame, &st); }
  *hash = mi_backtrace_hash(buffer, st.depth);  // no platform-provided hash, compute our own
  return st.depth;
}
#else
size_t _mi_pprof_backtrace(void** buffer, size_t max_depth, size_t* hash) {
  MI_UNUSED(buffer);
  MI_UNUSED(max_depth);
  *hash = 0;
  return 0;
}
#endif


// ---------------------------------------------------------------------------
// Write out mapped libraries
// ---------------------------------------------------------------------------

mi_decl_maybe_unused static void mi_pprof_modules_add(mi_heap_t* heap, mi_pprof_modules_t* mods, uintptr_t start, uintptr_t end, uint64_t file_offset, const char* path) {
  if (mods->failed) return;
  if (mods->count == mods->capacity) {
    const size_t new_capacity = (mods->capacity == 0 ? 16 : mods->capacity * 2);
    mi_pprof_module_t* const new_modules = (mi_pprof_module_t*)mi_heap_realloc(heap, mods->modules, new_capacity * sizeof(*mods->modules));
    if (new_modules == NULL) { mods->failed = true; return; }
    mods->modules = new_modules;
    mods->capacity = new_capacity;
  }
  char* const copy = (path != NULL ? mi_heap_strndup(heap, path, 1024) : NULL);
  if (path != NULL && copy == NULL) { mods->failed = true; return; }
  mi_pprof_module_t* m = &mods->modules[mods->count++];
  m->start = start;
  m->end = end;
  m->file_offset = file_offset;
  m->path = copy;
}

void _mi_pprof_modules_done(mi_pprof_modules_t* mods) {
  for (size_t i = 0; i < mods->count; i++) { mi_free(mods->modules[i].path); }
  mi_free(mods->modules);
  mods->modules = NULL;
  mods->count = 0;
  mods->capacity = 0;
}

// On Linux, `pprof` can symbolize addresses using the `MAPPED_LIBRARIES` trailer
// which is just a copy of `/proc/self/maps`. On macOS there is no equivalent
// file so we reconstruct a similarly formatted line (`start-end perm ... path`)
// per loaded Mach-O image using the dyld APIs: the range is taken from the
// image's `__TEXT` segment, which is also the "load address" that tools like
// `atos` use to symbolize addresses (`atos -o <path> -l <start> <addr>`). On
// Windows we walk the process address space with `VirtualQuery`
// On other platforms we leave this section empty (`pprof` can still work
// without it if not using PIE, or when combined with tools like `atos` /
// `addr2line` / a symbol server).

// A small wrapper that formats with our own minimal `_mi_vsnprintf` (so call
// sites can use its `%l8x`/`%l8d`/`%tx` extensions instead of <inttypes.h>'s
// `PRIx64`/`PRIu64`/`PRIxPTR` macros) and writes the result to a snapshot file.
// (Distinct from the existing `_mi_fprintf(mi_output_fun*, void*, ...)`  in
// `util/options.c`, which targets mimalloc's redirectable output callback instead.)
void _mi_pprof_fprintf(mi_pprof_file_t* f, const char* fmt, ...) {
  char buf[512];
  va_list args;
  va_start(args, fmt);
  _mi_vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  _mi_pprof_fputs(f, buf);
}

#if defined(_WIN32)
#include <windows.h>

// Walk the process address space, calling `on_module(ctx, base, end, path)`
// once for each loaded module. Consecutive `MEM_IMAGE` regions that belong to
// the same module (same allocation base) are merged into a single range.
static void mi_walk_modules_win32(void (*on_module)(void* ctx, uintptr_t base, uintptr_t end, const char* path), void* ctx) {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  uint8_t* addr = (uint8_t*)si.lpMinimumApplicationAddress;
  uint8_t* const addr_max = (uint8_t*)si.lpMaximumApplicationAddress;
  void* cur_base = NULL;
  uintptr_t cur_end = 0;
  while (addr < addr_max) {
    MEMORY_BASIC_INFORMATION mbi;
    const SIZE_T n = VirtualQuery(addr, &mbi, sizeof(mbi));
    if (n == 0) break;  // no more (queryable) regions
    if (mbi.Type == MEM_IMAGE && mbi.State != MEM_FREE) {
      if (mbi.AllocationBase == cur_base) {
        // extend the current module's range
        cur_end = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
      }
      else {
        // a new module starts: flush the previous one first
        if (cur_base != NULL) {
          char path[MAX_PATH];
          const DWORD len = GetModuleFileNameA((HMODULE)cur_base, path, (DWORD)sizeof(path));
          on_module(ctx, (uintptr_t)cur_base, cur_end, (len > 0 ? path : ""));
        }
        cur_base = mbi.AllocationBase;
        cur_end  = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
      }
    }
    else if (cur_base != NULL) {
      // the module's regions ended: flush it
      char path[MAX_PATH];
      const DWORD len = GetModuleFileNameA((HMODULE)cur_base, path, (DWORD)sizeof(path));
      on_module(ctx, (uintptr_t)cur_base, cur_end, (len > 0 ? path : ""));
      cur_base = NULL;
    }
    const uint8_t* next = (const uint8_t*)mbi.BaseAddress + mbi.RegionSize;
    if (next <= addr) break;  // guard against a non-advancing region
    addr = (uint8_t*)next;
  }
  if (cur_base != NULL) {
    char path[MAX_PATH];
    const DWORD len = GetModuleFileNameA((HMODULE)cur_base, path, (DWORD)sizeof(path));
    on_module(ctx, (uintptr_t)cur_base, cur_end, (len > 0 ? path : ""));
  }
}

static void mi_write_mapped_libraries_on_module_win32(void* ctx, uintptr_t base, uintptr_t end, const char* path) {
  mi_pprof_file_t* f = (mi_pprof_file_t*)ctx;
  _mi_pprof_fprintf(f, "%tx-%tx r-xp 00000000 00:00 0            %s\n", base, end, path);
}

static void mi_write_mapped_libraries_win32(mi_pprof_file_t* f) {
  mi_walk_modules_win32(&mi_write_mapped_libraries_on_module_win32, f);
}

typedef struct { mi_heap_t* heap; mi_pprof_modules_t* mods; } mi_pprof_collect_modules_win32_ctx_t;

static void mi_pprof_collect_modules_win32_on_module(void* ctx, uintptr_t base, uintptr_t end, const char* path) {
  mi_pprof_collect_modules_win32_ctx_t* c = (mi_pprof_collect_modules_win32_ctx_t*)ctx;
  mi_pprof_modules_add(c->heap, c->mods, base, end, 0, (path[0] != 0 ? path : NULL));
}

static void mi_pprof_collect_modules_win32(mi_heap_t* heap, mi_pprof_modules_t* mods) {
  mi_pprof_collect_modules_win32_ctx_t ctx = { heap, mods };
  mi_walk_modules_win32(&mi_pprof_collect_modules_win32_on_module, &ctx);
}

#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <mach-o/loader.h>

// Find the (slid) virtual address range of the `__TEXT` segment of a loaded
// Mach-O image; this is the image's load address as used for symbolication.
static bool mi_macho_image_range(const struct mach_header* mh, intptr_t slide, uintptr_t* start, uintptr_t* end) {
  const bool is64 = (mh->magic == MH_MAGIC_64 || mh->magic == MH_CIGAM_64);
  const uint8_t* cmd_ptr = (const uint8_t*)mh + (is64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header));
  for (uint32_t c = 0; c < mh->ncmds; c++) {
    const struct load_command* lc = (const struct load_command*)cmd_ptr;
    if (is64 && lc->cmd == LC_SEGMENT_64) {
      const struct segment_command_64* seg = (const struct segment_command_64*)lc;
      if (_mi_streq(seg->segname, SEG_TEXT)) {
        *start = (uintptr_t)seg->vmaddr + (uintptr_t)slide;
        *end   = *start + (uintptr_t)seg->vmsize;
        return true;
      }
    }
    else if (!is64 && lc->cmd == LC_SEGMENT) {
      const struct segment_command* seg = (const struct segment_command*)lc;
      if (_mi_streq(seg->segname, SEG_TEXT)) {
        *start = (uintptr_t)seg->vmaddr + (uintptr_t)slide;
        *end   = *start + (uintptr_t)seg->vmsize;
        return true;
      }
    }
    cmd_ptr += lc->cmdsize;
  }
  return false;  // no __TEXT segment found
}

static void mi_write_mapped_libraries_macos(mi_pprof_file_t* f) {
  const uint32_t count = _dyld_image_count();
  for (uint32_t i = 0; i < count; i++) {
    const struct mach_header* mh = _dyld_get_image_header(i);
    const char* name = _dyld_get_image_name(i);
    if (mh == NULL || name == NULL) continue;
    const intptr_t slide = _dyld_get_image_vmaddr_slide(i);
    uintptr_t start = 0;
    uintptr_t end = 0;
    if (!mi_macho_image_range(mh, slide, &start, &end)) continue;
    _mi_pprof_fprintf(f, "%tx-%tx r-xp 00000000 00:00 0            %s\n", start, end, name);
  }
}

static void mi_pprof_collect_modules_macos(mi_heap_t* heap, mi_pprof_modules_t* mods) {
  const uint32_t count = _dyld_image_count();
  for (uint32_t i = 0; i < count; i++) {
    const struct mach_header* mh = _dyld_get_image_header(i);
    const char* name = _dyld_get_image_name(i);
    if (mh == NULL || name == NULL) continue;
    const intptr_t slide = _dyld_get_image_vmaddr_slide(i);
    uintptr_t start = 0;
    uintptr_t end = 0;
    if (!mi_macho_image_range(mh, slide, &start, &end)) continue;
    mi_pprof_modules_add(heap, mods, start, end, 0, name);
  }
}

#elif defined(__linux__)
#include <errno.h>      // EINTR
#include <fcntl.h>      // open, O_RDONLY, O_CLOEXEC
#include <unistd.h>     // read, close

// Call `on_line(ctx, line)` for each line of `/proc/self/maps` (without using stdio, see `_mi_pprof_fopen`).
// The line is zero terminated and includes its newline; lines longer than 511 bytes are truncated.
static void mi_read_maps_linux(void (*on_line)(void* ctx, char* line), void* ctx) {
  int oflag = O_RDONLY;
  #if defined(O_CLOEXEC)
  oflag |= O_CLOEXEC;
  #endif
  const int fd = open("/proc/self/maps", oflag);
  if (fd < 0) return;
  char buf[4096];
  char line[512];
  size_t line_len = 0;
  for (;;) {
    const ssize_t n = read(fd, buf, sizeof(buf));
    if (n < 0 && errno == EINTR) continue;
    if (n <= 0) break;
    for (ssize_t i = 0; i < n; i++) {
      const char c = buf[i];
      if (c == '\n') {
        line[line_len++] = '\n';
        line[line_len] = 0;
        on_line(ctx, line);
        line_len = 0;
      }
      else if (line_len < sizeof(line) - 2) {  // leave room for the newline and terminator
        line[line_len++] = c;
      }
    }
  }
  if (line_len > 0) { line[line_len] = 0; on_line(ctx, line); }
  close(fd);
}

static void mi_write_mapped_libraries_on_line_linux(void* ctx, char* line) {
  _mi_pprof_fputs((mi_pprof_file_t*)ctx, line);
}

// Copy `/proc/self/maps` verbatim; this is exactly the format `pprof` expects.
static void mi_write_mapped_libraries_linux(mi_pprof_file_t* f) {
  mi_read_maps_linux(&mi_write_mapped_libraries_on_line_linux, f);
}

typedef struct { mi_heap_t* heap; mi_pprof_modules_t* mods; } mi_pprof_collect_modules_linux_ctx_t;

// Parse `/proc/self/maps` lines of the form
// `<start>-<end> <perms> <offset> <dev> <inode>  <path>` (path may be absent
// for anonymous mappings, which are skipped here since they are not useful
// for symbolization).
static void mi_pprof_collect_modules_on_line_linux(void* ctx, char* line) {
  mi_pprof_collect_modules_linux_ctx_t* const c = (mi_pprof_collect_modules_linux_ctx_t*)ctx;
  mi_heap_t* const heap = c->heap;
  mi_pprof_modules_t* const mods = c->mods;
  const size_t line_len = _mi_strlen(line);
  unsigned long long start = 0;
  unsigned long long end = 0;
  unsigned long long offset = 0;
  const char* end_ptr = NULL;
  if (!_mi_strntoull(line, line_len, 16, &end_ptr, &start) || *end_ptr != '-') return;
  end_ptr++;
  if (!_mi_strntoull(end_ptr, line_len - (size_t)(end_ptr - line), 16, &end_ptr, &end)) return;
  // Skip permissions, device, and inode; preserve the offset and the entire pathname.
  for (size_t field = 0; field < 4; field++) {
    while (*end_ptr == ' ') end_ptr++;
    if (*end_ptr == 0 || *end_ptr == '\n') return;
    if (field == 1) {
      if (!_mi_strntoull(end_ptr, line_len - (size_t)(end_ptr - line), 16, &end_ptr, &offset)) return;
    }
    else {
      while (*end_ptr != 0 && *end_ptr != ' ' && *end_ptr != '\n') end_ptr++;
    }
    if (*end_ptr != ' ' && *end_ptr != '\n' && *end_ptr != 0) return;
  }
  const char* path = end_ptr;
  while (*path == ' ') path++;
  size_t len = _mi_strlen(path);
  while (len > 0 && (path[len-1] == '\n' || path[len-1] == '\r')) { len--; }
  if (len == 0 || path[0] == '[') return;  // skip anonymous mappings like `[heap]`, `[stack]`
  char pathbuf[512];
  if (len >= sizeof(pathbuf)) { len = sizeof(pathbuf) - 1; }
  mi_memcpy(pathbuf, path, len);
  pathbuf[len] = 0;
  mi_pprof_modules_add(heap, mods, (uintptr_t)start, (uintptr_t)end, (uint64_t)offset, pathbuf);
}

static void mi_pprof_collect_modules_linux(mi_heap_t* heap, mi_pprof_modules_t* mods) {
  mi_pprof_collect_modules_linux_ctx_t ctx = { heap, mods };
  mi_read_maps_linux(&mi_pprof_collect_modules_on_line_linux, &ctx);
}

#elif defined(__FreeBSD__)
#include <sys/types.h>
#include <sys/sysctl.h> // sysctl, CTL_KERN, KERN_PROC, KERN_PROC_VMMAP
#include <sys/user.h>   // struct kinfo_vmentry
#include <unistd.h>     // getpid
#include <stddef.h>     // offsetof

// FreeBSD does not reliably provide Linux's `/proc/self/maps` (procfs is optional and rarely
// mounted by default); instead we use the `KERN_PROC_VMMAP` sysctl to enumerate the process's
// virtual memory mappings. This returns packed `kinfo_vmentry` records of `kve_structsize` bytes
// each, where the final `kve_path` field is truncated. We use the sysctl directly (instead of
// `kinfo_getvmmap`) to avoid a dependency on `libutil`.
// (DragonFly forked from FreeBSD before `KERN_PROC_VMMAP` was added and does not provide it.)
static void mi_walk_mappings_freebsd(void (*on_mapping)(void* ctx, uintptr_t start, uintptr_t end, const char* path), void* ctx) {
  int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_VMMAP, (int)getpid() };
  size_t len = 0;
  if (sysctl(mib, 4, NULL, &len, NULL, 0) != 0 || len == 0) return;
  len = len * 4 / 3;  // leave room for mappings added in the meantime
  char* const buf = (char*)mi_malloc(len);
  if (buf == NULL) return;
  if (sysctl(mib, 4, buf, &len, NULL, 0) == 0) {
    const size_t path_ofs = offsetof(struct kinfo_vmentry, kve_path);
    size_t ofs = 0;
    while (ofs + path_ofs < len) {
      const struct kinfo_vmentry* const kve = (const struct kinfo_vmentry*)(buf + ofs);
      const size_t size = (size_t)kve->kve_structsize;
      if (size <= path_ofs || size > len - ofs) break;  // end of the records (or corrupted)
      ofs += size;
      const char* const path = kve->kve_path;
      if (path[0] == 0) continue;  // skip anonymous mappings
      if (_mi_strnlen(path, size - path_ofs) == size - path_ofs) continue;  // path is not terminated within the record
      on_mapping(ctx, (uintptr_t)kve->kve_start, (uintptr_t)kve->kve_end, path);
    }
  }
  mi_free(buf);
}

static void mi_write_mapped_libraries_on_mapping_freebsd(void* ctx, uintptr_t start, uintptr_t end, const char* path) {
  _mi_pprof_fprintf((mi_pprof_file_t*)ctx, "%tx-%tx r-xp 00000000 00:00 0            %s\n", start, end, path);
}

static void mi_write_mapped_libraries_bsd(mi_pprof_file_t* f) {
  mi_walk_mappings_freebsd(&mi_write_mapped_libraries_on_mapping_freebsd, f);
}

typedef struct { mi_heap_t* heap; mi_pprof_modules_t* mods; } mi_pprof_collect_modules_freebsd_ctx_t;

static void mi_pprof_collect_modules_freebsd_on_mapping(void* ctx, uintptr_t start, uintptr_t end, const char* path) {
  mi_pprof_collect_modules_freebsd_ctx_t* c = (mi_pprof_collect_modules_freebsd_ctx_t*)ctx;
  mi_pprof_modules_add(c->heap, c->mods, start, end, 0, path);
}

static void mi_pprof_collect_modules_bsd(mi_heap_t* heap, mi_pprof_modules_t* mods) {
  mi_pprof_collect_modules_freebsd_ctx_t ctx = { heap, mods };
  mi_walk_mappings_freebsd(&mi_pprof_collect_modules_freebsd_on_mapping, &ctx);
}
#endif

void _mi_pprof_collect_modules(mi_heap_t* heap, mi_pprof_modules_t* mods) {
  mods->modules = NULL;
  mods->count = 0;
  mods->capacity = 0;
  mods->failed = false;
  #if defined(__linux__)
  mi_pprof_collect_modules_linux(heap, mods);
  #elif defined(_WIN32)
  mi_pprof_collect_modules_win32(heap, mods);
  #elif defined(__APPLE__)
  mi_pprof_collect_modules_macos(heap, mods);
  #elif defined(__FreeBSD__)
  mi_pprof_collect_modules_bsd(heap, mods);
  #else
  MI_UNUSED(heap);
  #endif
}

void _mi_pprof_write_mapped_libraries(mi_pprof_file_t* f) {
  _mi_pprof_fputs(f, "MAPPED_LIBRARIES:\n");
  #if defined(__linux__)
  mi_write_mapped_libraries_linux(f);
  #elif defined(_WIN32)
  mi_write_mapped_libraries_win32(f);
  #elif defined(__APPLE__)
  mi_write_mapped_libraries_macos(f);
  #elif defined(__FreeBSD__)
  mi_write_mapped_libraries_bsd(f);
  #endif
}

#endif  // MI_PROFILE_USE_BUILTIN
