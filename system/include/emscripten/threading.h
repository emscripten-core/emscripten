#ifndef __emscripten_threading_h__
#define __emscripten_threading_h__

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t emscripten_atomic_cas_u8(void/*uint8_t*/ *addr, uint8_t oldVal, uint8_t newVal);
uint16_t emscripten_atomic_cas_u16(void/*uint16_t*/ *addr, uint16_t oldVal, uint16_t newVal);
uint32_t emscripten_atomic_cas_u32(void/*uint32_t*/ *addr, uint32_t oldVal, uint32_t newVal);

uint8_t emscripten_atomic_load_u8(const void/*uint8_t*/ *addr);
uint16_t emscripten_atomic_load_u16(const void/*uint16_t*/ *addr);
uint32_t emscripten_atomic_load_u32(const void/*uint32_t*/ *addr);

uint8_t emscripten_atomic_store_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_store_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_store_u32(void/*uint32_t*/ *addr, uint32_t val);

void emscripten_atomic_fence();

uint8_t emscripten_atomic_add_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_add_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_add_u32(void/*uint32_t*/ *addr, uint32_t val);

uint8_t emscripten_atomic_sub_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_sub_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_sub_u32(void/*uint32_t*/ *addr, uint32_t val);

uint8_t emscripten_atomic_and_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_and_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_and_u32(void/*uint32_t*/ *addr, uint32_t val);

uint8_t emscripten_atomic_or_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_or_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_or_u32(void/*uint32_t*/ *addr, uint32_t val);

uint8_t emscripten_atomic_xor_u8(void/*uint8_t*/ *addr, uint8_t val);
uint16_t emscripten_atomic_xor_u16(void/*uint16_t*/ *addr, uint16_t val);
uint32_t emscripten_atomic_xor_u32(void/*uint32_t*/ *addr, uint32_t val);

int emscripten_futex_wait(void/*uint32_t*/ *addr, uint32_t val, double maxWaitNanoseconds);
int emscripten_futex_wake(void/*uint32_t*/ *addr, int count);
int emscripten_futex_wake_or_requeue(void/*uint32_t*/ *addr, int count, int cmpValue, void/*uint32_t*/ *addr2);

typedef union em_variant_val
{
	int i;
	float f;
	double d;
	void *vp;
	char *cp;
} em_variant_val;

typedef struct em_queued_call
{
	int function;
	int operationDone;
	em_variant_val args[8];
	em_variant_val returnValue;
} em_queued_call;

void emscripten_sync_run_in_main_thread(em_queued_call *call);
void *emscripten_sync_run_in_main_thread_1(int function, void *arg1);
void *emscripten_sync_run_in_main_thread_2(int function, void *arg1, void *arg2);
void *emscripten_sync_run_in_main_thread_3(int function, void *arg1, void *arg2, void *arg3);

// Returns 1 if the current thread is the thread that hosts the Emscripten runtime.
int emscripten_is_main_runtime_thread(void);

// Returns 1 if the current thread is the main browser thread.
int emscripten_is_main_browser_thread(void);

void emscripten_main_thread_process_queued_calls();

#define EM_DEFERRED_FOPEN 1
#define EM_DEFERRED_FGETS 2
#define EM_DEFERRED_FPUTS 3
#define EM_DEFERRED_FCLOSE 4
#define EM_DEFERRED_OPENDIR 5
#define EM_DEFERRED_CLOSEDIR 6
#define EM_DEFERRED_TELLDIR 7
#define EM_DEFERRED_SEEKDIR 8
#define EM_DEFERRED_REWINDDIR 9
#define EM_DEFERRED_READDIR_R 10
#define EM_DEFERRED_READDIR 11
#define EM_DEFERRED_UTIME 12
#define EM_DEFERRED_UTIMES 13
#define EM_DEFERRED_STAT 14
#define EM_DEFERRED_LSTAT 15
#define EM_DEFERRED_FSTAT 16
#define EM_DEFERRED_MKNOD 17
#define EM_DEFERRED_MKDIR 18
#define EM_DEFERRED_MKFIFO 19
#define EM_DEFERRED_CHMOD 20
#define EM_DEFERRED_FCHMOD 21
#define EM_DEFERRED_LCHMOD 22
#define EM_DEFERRED_UMASK 23
#define EM_DEFERRED_STATVFS 24
#define EM_DEFERRED_FSTATVFS 25
#define EM_DEFERRED_OPEN 26
#define EM_DEFERRED_CREAT 27
#define EM_DEFERRED_MKTEMP 28
#define EM_DEFERRED_MKSTEMP 29
#define EM_DEFERRED_MKDTEMP 30
#define EM_DEFERRED_FCNTL 31
#define EM_DEFERRED_POSIX_FALLOCATE 32
#define EM_DEFERRED_POLL 33
#define EM_DEFERRED_ACCESS 34
#define EM_DEFERRED_CHDIR 35
#define EM_DEFERRED_CHOWN 36
#define EM_DEFERRED_CHROOT 37
#define EM_DEFERRED_CLOSE 38
#define EM_DEFERRED_DUP 39
#define EM_DEFERRED_DUP2 40
#define EM_DEFERRED_FCHOWN 41
#define EM_DEFERRED_FCHDIR 42
#define EM_DEFERRED_CTERMID 43
#define EM_DEFERRED_CRYPT 44
#define EM_DEFERRED_ENCRYPT 45
#define EM_DEFERRED_FPATHCONF 46
#define EM_DEFERRED_FSYNC 47
#define EM_DEFERRED_TRUNCATE 48 
#define EM_DEFERRED_FTRUNCATE 49
#define EM_DEFERRED_GETCWD 50
#define EM_DEFERRED_GETWD 51
#define EM_DEFERRED_ISATTY 52
#define EM_DEFERRED_LCHOWN 53
#define EM_DEFERRED_LINK 54
#define EM_DEFERRED_LOCKF 55
#define EM_DEFERRED_LSEEK 56
#define EM_DEFERRED_PIPE 57
#define EM_DEFERRED_PREAD 58
#define EM_DEFERRED_READ 59
#define EM_DEFERRED_RMDIR 60
#define EM_DEFERRED_UNLINK 61
#define EM_DEFERRED_TTYNAME 62
#define EM_DEFERRED_TTYNAME_R 63
#define EM_DEFERRED_SYMLINK 64
#define EM_DEFERRED_READLINK 65
#define EM_DEFERRED_PWRITE 66
#define EM_DEFERRED_WRITE 67
#define EM_DEFERRED_CONFSTR 68
#define EM_DEFERRED_GETHOSTNAME 69
#define EM_DEFERRED_GETLOGIN 70
#define EM_DEFERRED_GETLOGIN_R 71
#define EM_DEFERRED_SYSCONF 72
#define EM_DEFERRED_SBRK 73
#define EM_DEFERRED_CLEARERR 74
#define EM_DEFERRED_FDOPEN 75
#define EM_DEFERRED_FEOF 76
#define EM_DEFERRED_FERROR 77
#define EM_DEFERRED_FFLUSH 78
#define EM_DEFERRED_FGETC 79
#define EM_DEFERRED_GETCHAR 80
#define EM_DEFERRED_FGETPOS 81
#define EM_DEFERRED_GETS 82
#define EM_DEFERRED_FILENO 83
#define EM_DEFERRED_FPUTC 84
#define EM_DEFERRED_PUTCHAR 85
#define EM_DEFERRED_PUTS 86
#define EM_DEFERRED_FREAD 87
#define EM_DEFERRED_FREOPEN 88
#define EM_DEFERRED_FSEEK 89
#define EM_DEFERRED_FSETPOS 90
#define EM_DEFERRED_FTELL 91
#define EM_DEFERRED_FWRITE 92
#define EM_DEFERRED_POPEN 93
#define EM_DEFERRED_PCLOSE 94
#define EM_DEFERRED_PERROR 95
#define EM_DEFERRED_REMOVE 96
#define EM_DEFERRED_RENAME 97
#define EM_DEFERRED_REWIND 98
#define EM_DEFERRED_TMPNAM 99
#define EM_DEFERRED_TEMPNAM 100
#define EM_DEFERRED_TMPFILE 101
#define EM_DEFERRED_UNGETC 102
#define EM_DEFERRED_FSCANF 103
#define EM_DEFERRED_SCANF 104
#define EM_DEFERRED_FPRINTF 105
#define EM_DEFERRED_PRINTF 106
#define EM_DEFERRED_DPRINTF 107
#define EM_DEFERRED_MMAP 108
#define EM_DEFERRED_MUNMAP 109
#define EM_DEFERRED_ATEXIT 110
#define EM_DEFERRED_GETENV 111
#define EM_DEFERRED_CLEARENV 112
#define EM_DEFERRED_SETENV 113
#define EM_DEFERRED_UNSETENV 114
#define EM_DEFERRED_PUTENV 115
#define EM_DEFERRED_REALPATH 116
#define EM_DEFERRED_TCGETATTR 117
#define EM_DEFERRED_TCSETATTR 118
#define EM_DEFERRED_TZSET 119
#define EM_DEFERRED_SOCKET 120
#define EM_DEFERRED_BIND 121
#define EM_DEFERRED_SENDMSG 122
#define EM_DEFERRED_RECVMSG 123
#define EM_DEFERRED_SHUTDOWN 124
#define EM_DEFERRED_IOCTL 125
#define EM_DEFERRED_ACCEPT 126
#define EM_DEFERRED_SELECT 127
#define EM_DEFERRED_CONNECT 128
#define EM_DEFERRED_LISTEN 129
#define EM_DEFERRED_GETSOCKNAME 130
#define EM_DEFERRED_GETPEERNAME 131
#define EM_DEFERRED_SEND 132
#define EM_DEFERRED_RECV 133
#define EM_DEFERRED_SENDTO 134
#define EM_DEFERRED_RECVFROM 135
#define EM_DEFERRED_GETSOCKOPT 136

#ifdef __cplusplus
}
#endif

#endif
