# Design Doc: Precise Futex Wakeups

- **Status**: Completed
- **Bug**: https://github.com/emscripten-core/emscripten/issues/26633

## Context
Historically, `emscripten_futex_wait` (in
`system/lib/pthread/emscripten_futex_wait.c`) relied on a periodic wakeup loop
for pthreads and the main runtime thread. This was done for two primary reasons:

1. **Thread Cancellation**: To check if the calling thread had been cancelled while it was blocked.
2. **Main Runtime Thread Events**: To allow the main runtime thread (even when not the main browser thread) to process its mailbox/event queue.

The old implementation used a 1ms wakeup interval for the main runtime
thread and a 100ms interval for cancellable pthreads. This led to unnecessary
CPU wakeups and increased latency for events.

## Goals
- Remove the periodic wakeup loop from `emscripten_futex_wait`.
- Implement precise, event-driven wakeups for cancellation and mailbox events.
- Maintain the existing `emscripten_futex_wait` API signature.
- Focus implementation on threads that support `atomic.wait` (pthreads and workers).

## Non-Goals
- **Main Browser Thread**: Changes to the busy-wait loop in `futex_wait_main_browser_thread` are out of scope.
- **Direct Atomics Usage**: Threads that call `atomic.wait` directly (bypassing `emscripten_futex_wait`) remain un-interruptible.
- **Wasm Workers**: Wasm Workers do not have a `pthread` structure, so they are not covered by this design.

## Design

The core idea is to allow "side-channel" wakeups (cancellation, mailbox events)
to interrupt the `atomic.wait` call by having the waker call `atomic.wake` on the
same address the waiter is currently blocked on.

As part of this design, `emscripten_futex_wait` now explicitly supports spurious
wakeups.  i.e. it may return `0` (success) even if the underlying futex was not
explicitly woken by the application.

### 1. `struct pthread` Extensions
A single atomic `wait_addr` field was added to `struct pthread` (in
`system/lib/libc/musl/src/internal/pthread_impl.h`).

```c
// The address the thread is currently waiting on in emscripten_futex_wait.
//
// This field encodes the state using the following bitmask:
// - NULL: Not waiting, no pending notification.
// - NOTIFY_BIT (0x1): Not waiting, but a notification was sent.
// - addr: Waiting on `addr`, no pending notification.
// - addr | NOTIFY_BIT: Waiting on `addr`, notification sent.
//
// Since futex addresses must be 4-byte aligned, the low bit is safe to use.
_Atomic uintptr_t wait_addr;

#define NOTIFY_BIT (1 << 0)
```

### 2. Waiter Logic (`emscripten_futex_wait`)
The waiter follows this logic:

1.  **Publish Wait Address**:
    ```c
    uintptr_t expected_null = 0;
    if (!atomic_compare_exchange_strong(&self->wait_addr, &expected_null, (uintptr_t)addr)) {
        // If the CAS failed, it means NOTIFY_BIT was set by another thread.
        assert(expected_null & NOTIFY_BIT);
        // We don't wait at all; instead behave as if we spuriously woke up.
        ret = ATOMICS_WAIT_OK;
        goto done;
    }
    ```
2.  **Wait**: Call `ret = __builtin_wasm_memory_atomic_wait32(addr, val, timeout)`.
3.  **Unpublish**:
    ```c
    done:
    self->wait_addr = 0;
    ```
4.  **Handle side effects**: If the wake was due to cancellation or mailbox
    events, these are handled after `emscripten_futex_wait` returns (or
    internally via `pthread_testcancel` if cancellable).

Note: We do **not** loop internally if `ret == ATOMICS_WAIT_OK`. Even if we
suspect the wake was caused by a side-channel event, we must return to the user
to avoid "swallowing" a simultaneous real application wake.

### 3. Waker Logic (`_emscripten_thread_notify`)
When a thread needs to wake another thread for a side-channel event (e.g.
enqueuing work or cancellation), it calls `_emscripten_thread_notify`:

```c
void _emscripten_thread_notify(pthread_t target) {
  uintptr_t addr = atomic_fetch_or(&target->wait_addr, NOTIFY_BIT);
  if (addr == 0 || (addr & NOTIFY_BIT) != 0) {
    // Either the thread wasn't waiting (it will see NOTIFY_BIT later),
    // or someone else is already in the process of notifying it.
    return;
  }
  // We set the bit and are responsible for waking the target.
  // The target is currently waiting on `addr`.
  while (target->wait_addr == (addr | NOTIFY_BIT)) {
    emscripten_futex_wake((void*)addr, INT_MAX);
    sched_yield();
  }
}
```

### 4. Handling the Race Condition
The protocol handles the "Lost Wakeup" race by having the waker loop until the
waiter clears its `wait_addr`. If the waker sets the `NOTIFY_BIT` just before
the waiter enters `atomic.wait`, the `atomic_wake` will be delivered once the
waiter is asleep. If the waiter wakes up for any reason (timeout, real wake, or
side-channel wake), its reset of `wait_addr` to `0` will satisfy the waker's
loop condition.

## Benefits

- **Lower Power Consumption**: Threads can sleep indefinitely (or for the full duration of a user-requested timeout) without periodic wakeups.
- **Lower Latency**: Mailbox events and cancellation requests are processed immediately rather than waiting for the next tick.
- **Simpler Loop**: The complex logic for calculating remaining timeout slices in `emscripten_futex_wait` was removed.

## Alternatives Considered
- **Signal-based wakeups**: Not currently feasible in Wasm as signals are not
  implemented in a way that can interrupt `atomic.wait`.
- **A single global "wake-up" address per thread**: This would require the
  waiter to wait on *two* addresses simultaneously (the user's futex and its
  own wakeup address), which `atomic.wait` does not support. The implemented
  design works around this by having the waker use the *user's* futex address.

## Security/Safety Considerations
- **The `wait_addr` is managed carefully** to ensure wakers don't
  call `atomic.wake` on stale addresses. Clearing the address upon wake
  mitigates this.
- **The waker loop has a yield** to prevent a busy-wait deadlock if the waiter
  is somehow prevented from waking up (though `atomic.wait` is generally
  guaranteed to wake if `atomic.wake` is called).
