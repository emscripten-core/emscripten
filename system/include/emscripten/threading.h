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

#define EM_PROXIED_FOPEN 1
#define EM_PROXIED_FGETS 2
#define EM_PROXIED_FPUTS 3
#define EM_PROXIED_FCLOSE 4
#define EM_PROXIED_OPENDIR 5
#define EM_PROXIED_CLOSEDIR 6
#define EM_PROXIED_TELLDIR 7
#define EM_PROXIED_SEEKDIR 8
#define EM_PROXIED_REWINDDIR 9
#define EM_PROXIED_READDIR_R 10
#define EM_PROXIED_READDIR 11
#define EM_PROXIED_UTIME 12
#define EM_PROXIED_UTIMES 13
#define EM_PROXIED_STAT 14
#define EM_PROXIED_LSTAT 15
#define EM_PROXIED_FSTAT 16
#define EM_PROXIED_MKNOD 17
#define EM_PROXIED_MKDIR 18
#define EM_PROXIED_MKFIFO 19
#define EM_PROXIED_CHMOD 20
#define EM_PROXIED_FCHMOD 21
#define EM_PROXIED_LCHMOD 22
#define EM_PROXIED_UMASK 23
#define EM_PROXIED_STATVFS 24
#define EM_PROXIED_FSTATVFS 25
#define EM_PROXIED_OPEN 26
#define EM_PROXIED_CREAT 27
#define EM_PROXIED_MKTEMP 28
#define EM_PROXIED_MKSTEMP 29
#define EM_PROXIED_MKDTEMP 30
#define EM_PROXIED_FCNTL 31
#define EM_PROXIED_POSIX_FALLOCATE 32
#define EM_PROXIED_POLL 33
#define EM_PROXIED_ACCESS 34
#define EM_PROXIED_CHDIR 35
#define EM_PROXIED_CHOWN 36
#define EM_PROXIED_CHROOT 37
#define EM_PROXIED_CLOSE 38
#define EM_PROXIED_DUP 39
#define EM_PROXIED_DUP2 40
#define EM_PROXIED_FCHOWN 41
#define EM_PROXIED_FCHDIR 42
#define EM_PROXIED_CTERMID 43
#define EM_PROXIED_CRYPT 44
#define EM_PROXIED_ENCRYPT 45
#define EM_PROXIED_FPATHCONF 46
#define EM_PROXIED_FSYNC 47
#define EM_PROXIED_TRUNCATE 48 
#define EM_PROXIED_FTRUNCATE 49
#define EM_PROXIED_GETCWD 50
#define EM_PROXIED_GETWD 51
#define EM_PROXIED_ISATTY 52
#define EM_PROXIED_LCHOWN 53
#define EM_PROXIED_LINK 54
#define EM_PROXIED_LOCKF 55
#define EM_PROXIED_LSEEK 56
#define EM_PROXIED_PIPE 57
#define EM_PROXIED_PREAD 58
#define EM_PROXIED_READ 59
#define EM_PROXIED_RMDIR 60
#define EM_PROXIED_UNLINK 61
#define EM_PROXIED_TTYNAME 62
#define EM_PROXIED_TTYNAME_R 63
#define EM_PROXIED_SYMLINK 64
#define EM_PROXIED_READLINK 65
#define EM_PROXIED_PWRITE 66
#define EM_PROXIED_WRITE 67
#define EM_PROXIED_CONFSTR 68
#define EM_PROXIED_GETHOSTNAME 69
#define EM_PROXIED_GETLOGIN 70
#define EM_PROXIED_GETLOGIN_R 71
#define EM_PROXIED_SYSCONF 72
#define EM_PROXIED_SBRK 73
#define EM_PROXIED_CLEARERR 74
#define EM_PROXIED_FDOPEN 75
#define EM_PROXIED_FEOF 76
#define EM_PROXIED_FERROR 77
#define EM_PROXIED_FFLUSH 78
#define EM_PROXIED_FGETC 79
#define EM_PROXIED_GETCHAR 80
#define EM_PROXIED_FGETPOS 81
#define EM_PROXIED_GETS 82
#define EM_PROXIED_FILENO 83
#define EM_PROXIED_FPUTC 84
#define EM_PROXIED_PUTCHAR 85
#define EM_PROXIED_PUTS 86
#define EM_PROXIED_FREAD 87
#define EM_PROXIED_FREOPEN 88
#define EM_PROXIED_FSEEK 89
#define EM_PROXIED_FSETPOS 90
#define EM_PROXIED_FTELL 91
#define EM_PROXIED_FWRITE 92
#define EM_PROXIED_POPEN 93
#define EM_PROXIED_PCLOSE 94
#define EM_PROXIED_PERROR 95
#define EM_PROXIED_REMOVE 96
#define EM_PROXIED_RENAME 97
#define EM_PROXIED_REWIND 98
#define EM_PROXIED_TMPNAM 99
#define EM_PROXIED_TEMPNAM 100
#define EM_PROXIED_TMPFILE 101
#define EM_PROXIED_UNGETC 102
#define EM_PROXIED_FSCANF 103
#define EM_PROXIED_SCANF 104
#define EM_PROXIED_FPRINTF 105
#define EM_PROXIED_PRINTF 106
#define EM_PROXIED_DPRINTF 107
#define EM_PROXIED_MMAP 108
#define EM_PROXIED_MUNMAP 109
#define EM_PROXIED_ATEXIT 110
#define EM_PROXIED_GETENV 111
#define EM_PROXIED_CLEARENV 112
#define EM_PROXIED_SETENV 113
#define EM_PROXIED_UNSETENV 114
#define EM_PROXIED_PUTENV 115
#define EM_PROXIED_REALPATH 116
#define EM_PROXIED_TCGETATTR 117
#define EM_PROXIED_TCSETATTR 118
#define EM_PROXIED_TZSET 119
#define EM_PROXIED_SOCKET 120
#define EM_PROXIED_BIND 121
#define EM_PROXIED_SENDMSG 122
#define EM_PROXIED_RECVMSG 123
#define EM_PROXIED_SHUTDOWN 124
#define EM_PROXIED_IOCTL 125
#define EM_PROXIED_ACCEPT 126
#define EM_PROXIED_SELECT 127
#define EM_PROXIED_CONNECT 128
#define EM_PROXIED_LISTEN 129
#define EM_PROXIED_GETSOCKNAME 130
#define EM_PROXIED_GETPEERNAME 131
#define EM_PROXIED_SEND 132
#define EM_PROXIED_RECV 133
#define EM_PROXIED_SENDTO 134
#define EM_PROXIED_RECVFROM 135
#define EM_PROXIED_GETSOCKOPT 136

#ifdef __cplusplus
}
#endif

#endif
