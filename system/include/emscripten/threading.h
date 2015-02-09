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

#ifdef __cplusplus
}
#endif

#endif
